#include "ParseCalcLine.h"

/*
	Copyright (c) 2006-2023 James W. Walker

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from
	the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.
*/

#include "autoCF.h"
#include "CalcException.hpp"
#include "CFStringToUTF8.h"
#include "DoAssign.hpp"
#include "DoBinaryFunc.hpp"
#include "DoBinOp.hpp"
#include "DoDefFunc.hpp"
#include "DoDefinedFunc.hpp"
#include "DoEvaluation.hpp"
#include "DoIf.hpp"
#include "DoNegate.hpp"
#include "DoUnaryFunc.hpp"
#include "SCalcState.hpp"
#include "SFixedSymbols.hpp"
#include "UTF8ToCFString.h"

#if __MACH__
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <CFDictionary.h>
	#include <CFNumber.h>
#endif

#include <boost/spirit/include/qi.hpp>

using namespace boost::spirit;
using namespace std;

#include <cmath>
#include <vector>

#define	DebugParse	0

#if DebugParse
	#include <iostream>
#endif

namespace
{
	typedef		std::vector<double>	 		DblStack;
	
	uint_parser<unsigned long long, 16> const
        bighex_p   = uint_parser<unsigned long long, 16>();
}


namespace
{	
	#pragma mark DebugAction
	struct DebugAction
	{
				DebugAction( const char* inText ) : mText( inText ) {}
				DebugAction( const DebugAction& inOther )
					: mText( inOther.mText ) {}
		
		inline void operator()(const char* inStart, const char* inEnd ) const;
		
		std::string	mText;
	};

	inline void DebugAction::operator()(const char* inStart, const char* inEnd ) const
	{
	#pragma unused( inStart, inEnd )
	#if DebugParse
		std::string	theMatch( inStart, inEnd );
		
		std::cout << "Matched " << mText.c_str() << " text '" <<
			theMatch.c_str() << "'." << std::endl;
	#endif
	}
	
	#pragma mark enum ECalcMode
	enum ECalcMode
	{
		kCalcMode_Evaluate = 0,
		kCalcMode_SyntaxCheckExpression
	};

	#pragma mark struct calculator
	struct calculator : public boost::spirit::grammar<calculator>
	{
					calculator( SCalcState& inState,
								ECalcMode inMode )
						: mState( inState ),
						mMode( inMode ) {}

		template <typename ScannerT>
		struct definition
		{
			definition( const calculator& self )
				: mMode( self.mMode )
			{
				identifier =
					lexeme_d[ alpha_p >> *(alnum_p | '_') ] -
						(
							self.mState.mFixed.mBinaryFuncs
						|
							self.mState.mFixed.mUnaryFuncs
						|
							"if"
						|
							self.mState.mFixed.mConstants
						);
				// Note: it is important that mBinaryFuncs precedes
				// mUnaryFuncs. Due to short-circuiting, the union
				// would otherwise never completely match atan2.
				// It would be possible to fix this by enclosing the
				// alternatives in longest_d.
				
				assignment =
					identifier[ assign(self.mState.mIdentifier) ]
					>> '=' >> expression;
				
				funcdef = identifier[ assign(self.mState.mFuncName) ]
					>> '(' >> identifier[ push_back_a(self.mState.mParamStack) ]
					>>	*(
							',' >> identifier[ push_back_a(self.mState.mParamStack) ]
						)
					>> ')' >> '='
					>> lexeme_d[*anychar_p][ assign(self.mState.mFuncDef) ];
					
				statement = (
					assignment[ DoAssign(self.mState) ]
					|
					funcdef[ DoDefFunc(self.mState) ]
					|
					expression[ DoEvaluation(self.mState) ]
					)
					>> end_p;
				
				factor
					=	lexeme_d[ str_p("0x") >> bighex_p[ append(self.mState.mValStack) ]]
					|	ureal_p[ append(self.mState.mValStack) ]
					|	'(' >> expression >> ')'
					|	(lexeme_d[self.mState.mFixed.mUnaryFuncs >> '('] >> expression
							>> ')')[ DoUnaryFunc(self.mState) ]
					|	(lexeme_d[self.mState.mFuncDefs >> '('] >> expression
							>> *( ',' >> expression )
							>> ')')[ DoDefinedFunc(self.mState) ]
					|	(lexeme_d[self.mState.mFixed.mBinaryFuncs >> '('] >> expression
							>> ',' >> expression >> ')')[ DoBinaryFunc(self.mState) ]
					|	( "if(" >> expression >> ','
							>> expressionNA[ assign(self.mState.mIf1) ] >> ','
							>> expressionNA[ assign(self.mState.mIf2) ] >> ')'
						)[ DoIf( self.mState ) ]
					|	self.mState.mVariables[ append(self.mState.mValStack) ]
					|	self.mState.mFixed.mConstants[ append(self.mState.mValStack) ]
					;
					// Note: The hex part of factor must come before the real part, otherwise ureal_p
					// will gobble the leading 0.
				
				factorNA
					=	lexeme_d[ str_p("0x") >> bighex_p ]
					|	ureal_p
					|	'(' >> expressionNA >> ')'
					|	(lexeme_d[self.mState.mFixed.mUnaryFuncs >> '('] >> expressionNA
							>> ')')
					|	(lexeme_d[self.mState.mFuncDefs >> '('] >> expressionNA
							>> *( ',' >> expressionNA )
							>> ')')
					|	(lexeme_d[self.mState.mFixed.mBinaryFuncs >> '('] >> expressionNA
							>> ',' >> expressionNA >> ')')
					|	"if(" >> expressionNA >> ',' >> expressionNA >> ','
							>> expressionNA >> ')'
					|	self.mState.mVariables
					|	self.mState.mFixed.mConstants
					;
				
				power = (
					factor >>
					!(
						('^' >> power)[ DoPower(self.mState) ][ DebugAction("^") ]
					)
					)[ DebugAction("power") ];
				
				powerNA = factorNA >> !( '^' >> powerNA );
				
				term = (
					power >>
					(
						*(
							power[ DoTimes(self.mState) ][ DebugAction("juxt") ]
						|	('*' >> power)[ DoTimes(self.mState) ][ DebugAction("times*") ]
						|	('/' >> power)[ DoDivide(self.mState) ][ DebugAction("div") ]
						)[ DebugAction("term-tail") ]
					)
					)[ DebugAction("term") ];
				
				termNA =
					powerNA >>
					*(
						powerNA
					|	('*' >> powerNA)
					|	('/' >> powerNA)
					);
				
				expression =
						(
							term
						|	('-' >> term)[ DoNegate(self.mState) ]
						)
						>>
						*(
							('+' >> term)[ DoPlus(self.mState) ][ DebugAction("+") ]
						|	('-' >> term)[ DoMinus(self.mState) ][ DebugAction("-") ]
						);
				
				expressionNA =
					( termNA | ('-' >> termNA ) )
					>>
					*(
						('+' >> termNA)
					|	('-' >> termNA)
					);
				
			}
			
			const boost::spirit::rule<ScannerT>& start() const
			{
				return (mMode == kCalcMode_Evaluate)? statement : expressionNA;
			}
			
			boost::spirit::rule<ScannerT>	factor, expression, term, power, identifier;
			boost::spirit::rule<ScannerT>	factorNA, expressionNA, termNA, powerNA;
			boost::spirit::rule<ScannerT>	assignment, statement, funcdef;
			ECalcMode		mMode;
		};
		
		SCalcState&		mState;
		ECalcMode		mMode;
	};
}

#pragma mark -

/*!
	@function	CheckExpressionSyntax
	@abstract	Check the syntax of an expression.
	@param		inLine		A NUL-terminated line of text.
	@param		inState		A calculator object reference.
	@param		outStop		Offset at which parsing stopped, which can be
							helpful in spotting a syntax error.
	@result		True if the expression was parsed successfully.
*/
bool	CheckExpressionSyntax( const char* inLine, CalcState inState,
					long* outStop )
{
	bool	isOK = false;
	*outStop = 0;
	
	try
	{
		calculator	theCalc( *inState, kCalcMode_SyntaxCheckExpression );
		inState->mValStack.clear();
		inState->mParamStack.clear();
		
		parse_info<>	parseResult = parse( inLine, theCalc, space_p );
		*outStop = parseResult.stop - inLine;
		
		if (parseResult.full)
		{
			isOK = true;
		}
	}
	catch (...)
	{
	}
	
	return isOK;
}

/*!
	@function	ParseCalcLine
	@abstract	Attempt to parse and compute an expression or assignment.
	@param		inLine		A NUL-terminated line of text.
	@param		ioState		A calculator object reference.
	@param		outValue	Receives the computed value if the computation
							succeeded.
	@param		outStop		Offset at which parsing stopped, which can be
							helpful in spotting a syntax error.
	@param		outSymbol	If a function was defined, this receives the name of
							the function.  If a variable was assigned, this
							receives the name of the variable.  Otherwise, this
							receives the empty string.
	@result		Whether we failed, calculated, or defined a function.
*/
ECalcResult		ParseCalcLine( const char* inLine, CalcState ioState,
						double* outValue, long* outStop, const char** outSymbol )
{
	ECalcResult	didParse = kCalcResult_Error;
	*outStop = 0;
	
	try
	{
		calculator	theCalc( *ioState, kCalcMode_Evaluate );
		ioState->mValStack.clear();
		ioState->mParamStack.clear();
		
		parse_info<>	parseResult = parse( inLine, theCalc, space_p );
		*outStop = parseResult.stop - inLine;
		
		if (parseResult.full)
		{
			if (ioState->mDidDefineFunction)
			{
				didParse = kCalcResult_DefinedFunction;
			}
			else if (ioState->mValStack.size() >= 1)
			{
				*outValue = ioState->mValStack.back();
				didParse = kCalcResult_Calculated;
			
			#if DebugParse
				std::cout << "Stack: ";
				for (DblStack::iterator i = ioState->mValStack.begin();
					i != ioState->mValStack.end(); ++i)
				{
					std::cout << *i << " ";
				}
				std::cout << std::endl;
			#endif
			}
			
			if (outSymbol != NULL)
			{
				*outSymbol = ioState->mDefinedSymbol.c_str();
			}
		}
	}
	catch (...)
	{
		#if DebugParse
			std::cout << "Exception thrown." << std::endl;
		#endif
		ioState->mDidDefineFunction = false;
	}
	return didParse;
}


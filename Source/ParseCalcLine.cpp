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
#include "DoAppendNumber.hpp"
#include "DoAssign.hpp"
#include "DoBinaryFunc.hpp"
#include "DoBinOp.hpp"
#include "DoDefFunc.hpp"
#include "DoDefinedFunc.hpp"
#include "DoEvaluation.hpp"
#include "DoGetMatchedString.hpp"
#include "DoIf.hpp"
#include "DoNegate.hpp"
#include "DoPushMatchedString.hpp"
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

using namespace boost::spirit::qi;

#include <cmath>
#include <vector>

#define	DebugParse	0

#if DebugParse
	#include <iostream>
#endif

namespace
{
	uint_parser<unsigned long long, 16> const
        bighex_p   = uint_parser<unsigned long long, 16>();
}


namespace
{	
	#pragma mark enum ECalcMode
	enum ECalcMode
	{
		kCalcMode_Evaluate = 0,
		kCalcMode_SyntaxCheckExpression
	};

	#pragma mark struct calculator
	template <typename Iterator>
	struct calculator : public grammar<Iterator, ascii::space_type>
	{
			calculator( SCalcState& inState,
							ECalcMode inMode )
				: calculator::base_type( start )
				, mState( inState )
				, mMode( inMode )
			{
				identifier =
					lexeme[ standard::alpha >> *(standard::alnum | '_') ] -
						(
							mState.mFixed.mBinaryFuncs
						|
							mState.mFixed.mUnaryFuncs
						|
							"if"
						|
							mState.mFixed.mConstants
						);
				// Note: it is important that mBinaryFuncs precedes
				// mUnaryFuncs. Due to short-circuiting, the union
				// would otherwise never completely match atan2.
				// It would be possible to fix this by enclosing the
				// alternatives in longest_d.
				
				assignment =
					raw[identifier][ DoGetMatchedString(mState.mIdentifier) ]
					>> '=' >> expression;
				
				funcdef = raw[identifier][ DoGetMatchedString(mState.mFuncName) ]
					>> '(' >> raw[identifier][ DoPushMatchedString(mState) ]
					>>	*(
							',' >> raw[identifier][ DoPushMatchedString(mState) ]
						)
					>> ')' >> '='
					>> raw[lexeme[*standard::char_]][ DoGetMatchedString(mState.mFuncDef) ];
					
				statement = (
					assignment[ DoAssign(mState) ]
					|
					funcdef[ DoDefFunc(mState) ]
					|
					expression[ DoEvaluation(mState) ]
					)
					>> eoi;
				
				factor
					=	lexeme[ lit("0x") >> bighex_p[ DoAppendNumber(mState) ]]
					|	ureal[ DoAppendNumber(mState) ]
					|	'(' >> expression >> ')'
					|	raw[ lexeme[mState.mFixed.mUnaryFuncs >> '('] >> expression
							>> ')'][ DoUnaryFunc(mState) ]
					|	raw[lexeme[mState.mFuncDefs >> '('] >> expression
							>> *( ',' >> expression )
							>> ')'][ DoDefinedFunc(mState) ]
					|	raw[lexeme[mState.mFixed.mBinaryFuncs >> '('] >> expression
							>> ',' >> expression >> ')'][ DoBinaryFunc(mState) ]
					|	( "if(" >> expression >> ','
							>> raw[expressionNA][ DoGetMatchedString(mState.mIf1) ] >> ','
							>> raw[expressionNA][ DoGetMatchedString(mState.mIf2) ] >> ')'
						)[ DoIf( mState ) ]
					|	mState.mVariables[ DoAppendNumber(mState) ]
					|	mState.mFixed.mConstants[ DoAppendNumber(mState) ]
					;
					// Note: The hex part of factor must come before the real part, otherwise ureal_p
					// will gobble the leading 0.
				
				factorNA
					=	lexeme[ lit("0x") >> bighex_p ]
					|	ureal
					|	'(' >> expressionNA >> ')'
					|	(lexeme[mState.mFixed.mUnaryFuncs >> '('] >> expressionNA
							>> ')')
					|	(lexeme[mState.mFuncDefs >> '('] >> expressionNA
							>> *( ',' >> expressionNA )
							>> ')')
					|	(lexeme[mState.mFixed.mBinaryFuncs >> '('] >> expressionNA
							>> ',' >> expressionNA >> ')')
					|	"if(" >> expressionNA >> ',' >> expressionNA >> ','
							>> expressionNA >> ')'
					|	mState.mVariables
					|	mState.mFixed.mConstants
					;
				
				power = (
					factor >>
					!(
						('^' >> power)[ DoPower(mState) ]
					)
					);
				
				powerNA = factorNA >> !( '^' >> powerNA );
			
				term = (
					power >>
					(
						*(
							power[ DoTimes(mState) ]
						|	('*' >> power)[ DoTimes(mState) ]
						|	('/' >> power)[ DoDivide(mState) ]
						)
					)
					);
				
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
						|	('-' >> term)[ DoNegate(mState) ]
						)
						>>
						*(
							('+' >> term)[ DoPlus(mState) ]
						|	('-' >> term)[ DoMinus(mState) ]
						);
				
				expressionNA =
					( termNA | ('-' >> termNA ) )
					>>
					*(
						('+' >> termNA)
					|	('-' >> termNA)
					);
				
				start = (mMode == kCalcMode_Evaluate)? statement : expressionNA;
			}
			
			real_parser< double, ureal_policies<double> > ureal;
			
			rule<Iterator, ascii::space_type>	factor, expression, term, power, identifier;
			rule<Iterator, ascii::space_type>	factorNA, expressionNA, termNA, powerNA;
			rule<Iterator, ascii::space_type>	assignment, statement, funcdef;
			rule<Iterator, ascii::space_type>	start;
		
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
		calculator<const char*>	theCalc( *inState, kCalcMode_SyntaxCheckExpression );
		inState->mValStack.clear();
		inState->mParamStack.clear();
		
		const char* startIter = inLine;
		const char* endIter = inLine + strlen(inLine);

		bool success = phrase_parse( startIter, endIter, theCalc, ascii::space );
		*outStop = startIter - inLine;
		
		if (success and (startIter == endIter))
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
		calculator<const char*>	theCalc( *ioState, kCalcMode_Evaluate );
		ioState->mValStack.clear();
		ioState->mParamStack.clear();
		
		const char* startIter = inLine;
		const char* endIter = inLine + strlen(inLine);

		bool success = phrase_parse( startIter, endIter, theCalc, ascii::space );
		*outStop = startIter - inLine;
		
		if (success and (startIter == endIter))
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


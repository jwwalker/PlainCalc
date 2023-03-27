#include "ParseCalcLine.h"

/*
	Copyright (c) 2006-2015 James W. Walker

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
#include "SCalcState.hpp"
#include "SFixedSymbols.hpp"
#include "UTF8ToCFString.h"

#if __MACH__
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <CFDictionary.h>
	#include <CFNumber.h>
#endif

#define _MSL_ 1
#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 1

#pragma warn_unusedarg	off
#if !defined(BOOST_SPIRIT_USE_OLD_NAMESPACE)
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#endif
#include "boost/spirit/include/classic.hpp"
#pragma warn_unusedarg	reset

using namespace boost::spirit;
using namespace std;

#include <cmath>
#include <vector>
#include <exception>
#include <set>

#define	DebugParse	0

#if DebugParse
	#include <iostream>
#endif

#define	ThrowIfCFFail_( x ) do { if ((x) == NULL) throw std::bad_alloc(); } while (false)

#define	ThrowIfEmpty_( x ) do { if (x.empty()) throw CalcException(); } while (false)

static double rad( double inDegrees )
{
	return inDegrees * (M_PI / 180.0);
}

static double deg( double inRadians )
{
	return inRadians * (180.0 / M_PI);
}

namespace
{
	typedef		std::set< std::string >		StringSet;
	
	typedef		std::vector<double>	 		DblStack;
	typedef		std::vector< std::string >	StringVec;
	
	typedef		double (*UnaryFunc)( double );
	typedef		double (*BinaryFunc)( double, double );
	
	typedef		std::pair< StringVec, std::string >		FuncDef;
	
	uint_parser<unsigned long long, 16> const
        bighex_p   = uint_parser<unsigned long long, 16>();
}



#if DebugParse
inline void DumpStack( const DblStack& inStack, const char* inTag )
{
	std::cout << inTag << ": ";
	for (DblStack::const_iterator i = inStack.begin(); i != inStack.end(); ++i)
	{
		std::cout << *i << ", ";
	}
	std::cout << std::endl;
}
#endif

namespace
{
	#pragma mark BinFunctor
	/*!
		@function	BinFunctor
		@abstract	Functor template that converts a binary function to a
					binary functor.
	*/
	template <BinaryFunc F>
	struct BinFunctor
	{
		double	operator()( double inLHS, double inRHS ) const
				{
					return F( inLHS, inRHS );
				}
	};
	
	#pragma mark DoBinOp
	/*!
		@class		DoBinOp
		@abstract	Functor template for a semantic action that computes a
					binary operation.
		@discussion	The template parameter Op should be a class with a
					method of the signature:
					
					double	operator()( double inLHS, double inRHS ) const;
	*/
	template <class Op>
	struct DoBinOp
	{
				DoBinOp( SCalcState& ioState ) : mState( ioState ) {}
				DoBinOp( const DoBinOp& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char*, const char* ) const
				{
					ThrowIfEmpty_( mState.mValStack );
					double	rhs = mState.mValStack.back();
					mState.mValStack.pop_back();
					ThrowIfEmpty_( mState.mValStack );
					double	lhs = mState.mValStack.back();
					mState.mValStack.pop_back();
					double	theVal = Op()( lhs, rhs );
					mState.mValStack.push_back( theVal );
				}
		
		SCalcState&		mState;
	};
	

	#pragma mark DoBinOp typedefs
	typedef	DoBinOp< std::plus<double> >		DoPlus;
	typedef	DoBinOp< std::minus<double> >		DoMinus;
	typedef	DoBinOp< std::multiplies<double> >	DoTimes;
	typedef	DoBinOp< std::divides<double> >		DoDivide;
	typedef	DoBinOp< BinFunctor<std::pow> >		DoPower;


	#pragma mark DoUnaryFunc
	/*!
		@function	DoUnaryFunc
		@abstract	Functor for a semantic action that computes a unary function
					stored in a symbol table.
	*/
	struct DoUnaryFunc
	{
				DoUnaryFunc( SCalcState& ioState ) : mState( ioState ) {}
				DoUnaryFunc( const DoUnaryFunc& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char* inStart, const char* inEnd ) const
				{
					std::string	parsedText( inStart, inEnd );
					std::string::size_type	parenLoc = parsedText.find( '(' );
					if (parenLoc == std::string::npos)
					{
						throw CalcException();
					}
					parsedText.erase( parenLoc );
					UnaryFunc*	foundFunc = find( mState.mFixed.mUnaryFuncs, parsedText.c_str() );
					if (foundFunc == NULL)
					{
						throw CalcException();
					}
					mState.mValStack.back() = (*foundFunc)( mState.mValStack.back() );
				}
		
		
		SCalcState&		mState;
	};
	
	#pragma mark DoDefinedFunc
	struct DoDefinedFunc
	{
				DoDefinedFunc( SCalcState& ioState ) : mState( ioState ) {}
				DoDefinedFunc( const DoDefinedFunc& inOther ) : mState( inOther.mState ) {}
				
		void	operator()( const char* inStart, const char* inEnd ) const
				{
					// Find the name of the function
					std::string	parsedText( inStart, inEnd );
					std::string::size_type	parenLoc = parsedText.find( '(' );
					if (parenLoc == std::string::npos)
					{
						throw CalcException();
					}
					parsedText.erase( parenLoc );
					
					// Find the definition
					FuncDef*	foundFunc = find( mState.mFuncDefs, parsedText.c_str() );
					if (foundFunc == NULL)
					{
						throw CalcException();
					}
					
					// Set values of the formal parameters in a temporary state
					const StringVec&	formalParams( foundFunc->first );
					if (mState.mValStack.size() < formalParams.size())
					{
						throw CalcException();
					}
					SCalcState	tempState( mState );
					for (StringVec::const_reverse_iterator i = formalParams.rbegin();
						i != formalParams.rend(); ++i)
					{
						const std::string&	theParam( *i );
						tempState.SetVariable( theParam.c_str() );
						tempState.mValStack.pop_back();
						mState.mValStack.pop_back();
					}
					
					// Evaluate
					double	funcVal;
					long	stopOff;
					ECalcResult	didParse = ParseCalcLine( foundFunc->second.c_str(),
						&tempState, &funcVal, &stopOff );
					if (didParse == kCalcResult_Calculated)
					{
						mState.mValStack.push_back( funcVal );
					}
					else
					{
						throw CalcException();
					}
				}
		
		SCalcState&		mState;
	};
	
	#pragma mark DoNegate
	/*!
		@function	DoNegate
		@abstract	Functor for a semantic action that negates the value on the
					stack.
	*/
	struct DoNegate
	{
				DoNegate( SCalcState& ioState ) : mState( ioState ) {}
				DoNegate( const DoNegate& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char*, const char* ) const
				{
					mState.mValStack.back() = - mState.mValStack.back();
				}
		
		SCalcState&		mState;
	};
	
	#pragma mark DoBinaryFunc
	/*!
		@function	DoBinaryFunc
		@abstract	Functor for a semantic action that computes a binary function
					stored in a symbol table.
	*/
	struct DoBinaryFunc
	{
				DoBinaryFunc( SCalcState& ioState ) : mState( ioState ) {}
				DoBinaryFunc( const DoBinaryFunc& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char* inStart, const char* inEnd ) const
				{
					std::string	parsedText( inStart, inEnd );
					std::string::size_type	parenLoc = parsedText.find( '(' );
					if (parenLoc == std::string::npos)
					{
						throw CalcException();
					}
					parsedText.erase( parenLoc );
					BinaryFunc*	foundFunc = find( mState.mFixed.mBinaryFuncs, parsedText.c_str() );
					if (foundFunc == NULL)
					{
						throw CalcException();
					}
					ThrowIfEmpty_( mState.mValStack );
					double	param2 = mState.mValStack.back();
					mState.mValStack.pop_back();
					ThrowIfEmpty_( mState.mValStack );
					double	param1 = mState.mValStack.back();
					mState.mValStack.pop_back();
					mState.mValStack.push_back( (*foundFunc)( param1, param2 ) );
				}
		
		
		SCalcState&		mState;
	};

	
	#pragma mark DoIf
	/*!
		@function	DoIf
		@abstract	Functor for a semantic action that computes a ternary if.
	*/
	struct DoIf
	{
				DoIf( SCalcState& ioState ) : mState( ioState ) {}
				DoIf( const DoIf& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char* , const char*  ) const
				{
					ThrowIfEmpty_( mState.mValStack );
					double	condition = mState.mValStack.back();
					mState.mValStack.pop_back();
					
					std::string	valueStr;
					if (condition > 0.0)
					{
						valueStr = mState.mIf1;
					}
					else
					{
						valueStr = mState.mIf2;
					}
					
					double	funcVal;
					long	stopOff;
					SCalcState	tempState( mState );
					ECalcResult	didParse = ParseCalcLine( valueStr.c_str(), &tempState,
						&funcVal, &stopOff );
					if (didParse == kCalcResult_Calculated)
					{
						mState.mValStack.push_back( funcVal );
					}
					else
					{
						throw CalcException();
					}
				}
		
		
		SCalcState&		mState;
	};

	
	#pragma mark DoAssign
	/*!
		@function	DoAssign
		@abstract	Functor for a semantic action that assigns the value of
					an expression to a variable.
	*/
	struct DoAssign
	{
				DoAssign( SCalcState& ioState ) : mState( ioState ) {}
				DoAssign( const DoAssign& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char*, const char* ) const
				{
					mState.SetVariable( mState.mIdentifier.c_str() );
					mState.SetVariable( "last" );
					mState.mDidDefineFunction = false;
					mState.mDefinedSymbol = mState.mIdentifier;
				}
		
		SCalcState&		mState;
	};
	
	#pragma mark DoDefFunc
	struct DoDefFunc
	{
				DoDefFunc( SCalcState& ioState ) : mState( ioState ) {}
				DoDefFunc( const DoDefFunc& inOther )
					: mState( inOther.mState ) {}
		
		void	operator()( const char*, const char* ) const
				{
					// Before committing to the function, syntax check it.
					SCalcState	tempState( mState );
					tempState.SetFunc( mState.mFuncName, mState.mParamStack,
						mState.mFuncDef );

					// Find the definition
					FuncDef*	foundFunc = find( tempState.mFuncDefs,
						mState.mFuncName.c_str() );
					if (foundFunc == NULL)
					{
						throw CalcException();
					}

					// Set values of the formal parameters in the temporary
					// state.  It does not matter what I set them to, they
					// just need to be recognized as variables.
					const StringVec&	formalParams( foundFunc->first );
					tempState.mValStack.push_back( 1.0 );
					for (StringVec::const_iterator i = formalParams.begin();
						i != formalParams.end(); ++i)
					{
						const std::string&	theParam( *i );
						tempState.SetVariable( theParam.c_str() );
					}

					long	stopOff;
					if (CheckExpressionSyntax( foundFunc->second.c_str(),
						&tempState, &stopOff ))
					{
						mState.SetFunc( mState.mFuncName, mState.mParamStack,
							mState.mFuncDef );
						mState.mParamStack.clear();
						mState.mDidDefineFunction = true;
						mState.mDefinedSymbol = mState.mFuncName;
					}
					else
					{
						throw CalcException();
					}
				}
		
		SCalcState&		mState;
	};
	
	#pragma mark DoEvaluation
	/*!
		@function	DoEvaluation
		@abstract	Functor for a semantic action that assigns the value of
					an expression to the built-in variable "last".
	*/
	struct DoEvaluation
	{
				DoEvaluation( SCalcState& ioState ) : mState( ioState ) {}
				DoEvaluation( const DoEvaluation& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char*, const char* ) const
				{
					mState.SetVariable( "last" );
					mState.mDidDefineFunction = false;
					mState.mDefinedSymbol.clear();
				}
		
		SCalcState&		mState;
	};
	
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
CalcState	CreateCalcState()
{
	CalcState	theState = NULL;
	
	try
	{
		theState = new SCalcState;
	}
	catch (...)
	{
	}
	
	return theState;
}

void		DisposeCalcState( CalcState ioState )
{
	delete ioState;
}

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

/*!
	@function	CopyCalcVariables
	@abstract	Create a dictionary containing the variables (as CFStringRef keys)
				and their values (as CFNumberRef values).
	@param		inState		A calculator object reference.
	@result		A dictionary reference, or NULL on failure.
*/
CFDictionaryRef	CopyCalcVariables( CalcState inState )
{
	CFDictionaryRef	theDict = NULL;
	
	try
	{
		theDict = inState->CopyVariables();
	}
	catch (...)
	{
	}
	return theDict;
}

/*!
	@function	SetCalcVariables
	@abstract	Add values of variable to a calculator.
	@param		inDict		A dictionary containing variable names (as
							CFStringRef keys) and their values (as CFNumberRef
							values).
	@param		ioState		A calculator object reference.
*/
void			SetCalcVariables( CFDictionaryRef inDict, CalcState ioState )
{
	try
	{
		ioState->SetVariables( inDict );
	}
	catch (...)
	{
	}
}

/*!
	@function	SetCalcVariable
	@abstract	Add or change a variable value in a calculator.
	@param		inVarName	Name of a variable (UTF-8).
	@param		inValue		Value to be assigned to the variable.
	@param		ioState		A calculator object reference.
*/
void			SetCalcVariable( const char* inVarName, double inValue,
								CalcState ioState )
{
	try
	{
		ioState->mValStack.push_back( inValue );
		ioState->SetVariable( inVarName );
		ioState->mValStack.pop_back();
	}
	catch (...)
	{
	}
}

/*!
	@function	CopyCalcFunctions
	@abstract	Create a dictionary recording user-defined functions.
	@discussion	The keys of the dictionary are the function names.  Each value
				is a CFArrray of CFStrings, being the function definition
				followed by the formal parameters.
	@param		inState		A calculator object reference.
	@result		A dictionary reference, or NULL on failure.
*/
CFDictionaryRef	CopyCalcFunctions( CalcState inState )
{
	CFDictionaryRef	theDict = NULL;
	
	try
	{
		theDict = inState->CopyFuncDefs();
	}
	catch (...)
	{
	}
	return theDict;
}

/*!
	@function	SetCalcFunctions
	@abstract	Add defined functions to the calculator.
	@discussion	The keys of the dictionary are the function names.  Each value
				is a CFArrray of CFStrings, being the function definition
				followed by the formal parameters.
	@param		inDict		A dictionary recording functions.
	@param		inState		A calculator object reference.
*/
void			SetCalcFunctions( CFDictionaryRef inDict, CalcState ioState )
{
	try
	{
		ioState->SetFuncDefs( inDict );
	}
	catch (...)
	{
	}
}

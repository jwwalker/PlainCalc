#include "ParseCalcLine.h"


#include "autoCF.h"
#include "CFStringToUTF8.h"
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
#include "boost/spirit.hpp"
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

namespace
{
	typedef		std::set< std::string >		StringSet;
	
	typedef		std::vector<double>	 		DblStack;
	
	typedef		double (*UnaryFunc)( double );
	typedef		double (*BinaryFunc)( double, double );
	
	typedef		std::pair< std::string, std::string >	UnaryFuncDef;
	
	class CalcException : public std::exception
	{
	public:
		CalcException() throw() {}
		CalcException(const CalcException& inOther) throw()
				: std::exception( static_cast<const std::exception&>( inOther ) )
				{}
		CalcException& operator=(const CalcException& inOther ) throw()
				{
					std::exception::operator=( static_cast<const std::exception&>( inOther ) );
					return *this;
				}
	};
	
	struct SFixedSymbols
	{
								SFixedSymbols();
								
		symbols<UnaryFunc>		mUnaryFuncs;
		symbols<BinaryFunc>		mBinaryFuncs;
		symbols<double>			mConstants;
	};
}


SFixedSymbols::SFixedSymbols()
{
	mUnaryFuncs.add
		( "atan", std::atan )
		( "acos", std::acos )
		( "asin", std::asin )
		( "sin", std::sin )
		( "cos", std::cos )
		( "tan", std::tan )
		( "ceil", std::ceil )
		( "floor", std::floor )
		( "round", round )
		( "fabs", std::fabs )
		( "abs", std::fabs )
		( "log", std::log )
		( "log10", std::log10 )
		( "log2", log2 )
		( "exp", std::exp )
		( "sqrt", std::sqrt )
		( "Ã", std::sqrt )
		( "\xE2\x88\x9A", std::sqrt )	// sqrt character in UTF-8
		;
	
	mBinaryFuncs.add
		( "atan2", std::atan2 )
		( "max", fmax )
		( "min", fmin )
		;
	
	double	piVal = 4.0 * std::atan( 1.0 );
	
	mConstants.add
		( "pi", piVal )
		( "¹", piVal )
		( "\xCF\x80", piVal )	// the pi character in UTF-8
		( "e", std::exp(1.0) )
		;
}

static const SFixedSymbols&	GetFixedSyms()
{
	static const SFixedSymbols*	sSyms = NULL;
	if (sSyms == NULL)
	{
		sSyms = new SFixedSymbols;
	}
	return *sSyms;
}

#pragma mark struct SCalcState
struct SCalcState
{
							SCalcState();
							SCalcState( const SCalcState& inOther );
						
	void					SetVariable( const char* inVarName );
	void					SetFunc( const std::string& inFuncName,
									const std::string& inFormalParam,
									const std::string& inValue );
	
	CFDictionaryRef			CopyVariables() const;
	void					SetVariables( CFDictionaryRef inDict );
	
	const SFixedSymbols&	mFixed;
	
	DblStack				mValStack;
	symbols<UnaryFuncDef>	mUnaryFuncDefs;
	symbols<double>			mVariables;
	std::string				mIdentifier;
	StringSet				mVariableSet;
	std::string				mFuncName;
	std::string				mFuncParam;
	std::string				mFuncDef;
	std::string				mIf1;
	std::string				mIf2;
	bool					mDidDefineFunction;
};


SCalcState::SCalcState()
	: mFixed( GetFixedSyms() )
	, mDidDefineFunction( false )
{
}

SCalcState::SCalcState( const SCalcState& inOther )
	: mFixed( inOther.mFixed )
	, mValStack( inOther.mValStack )
	, mUnaryFuncDefs( inOther.mUnaryFuncDefs )
	, mVariables( inOther.mVariables )
	, mIdentifier( inOther.mIdentifier )
	, mVariableSet( inOther.mVariableSet )
	, mFuncName( inOther.mFuncName )
	, mFuncParam( inOther.mFuncParam )
	, mFuncDef( inOther.mFuncDef )
	, mIf1( inOther.mIf1 )
	, mIf2( inOther.mIf2 )
	, mDidDefineFunction( inOther.mDidDefineFunction )
{
}

void	SCalcState::SetVariable( const char* inVarName )
{
	double*	foundVar = find( mVariables, inVarName );
	if (foundVar == NULL)
	{
		mVariables.add( inVarName, mValStack.back() );
		mVariableSet.insert( std::string(inVarName) );
	}
	else
	{
		*foundVar = mValStack.back();
	}
}

static bool IsIdentifierChar( char inChar )
{
	return isalnum( inChar ) or (inChar == '_');
}

/*!
	@function	FixFormalParam
	@abstract	Prefix the formal parameter by an underscore, so it cannot be
				confused with a normal variable.
*/
static void FixFormalParam( UnaryFuncDef& ioDef )
{
	std::string::size_type	startOff = 0;
	std::string::size_type	foundOff, identStart, identEnd;
	
	while ( (foundOff = ioDef.second.find( ioDef.first, startOff )) != std::string::npos )
	{
		// We found a substring that looks like the formal parameter, but we
		// must be sure that it is not just part of some other identifier.
		identStart = foundOff;
		identEnd = identStart + ioDef.first.size();
		if ( (identEnd == ioDef.second.size()) or (not IsIdentifierChar(ioDef.second[identEnd])) )
		{
			// Looking to the left is trickier, e.g., while 12x is not an
			// identifier, A12x is.
			while ( (identStart > 0) and IsIdentifierChar(ioDef.second[ identStart - 1 ]) )
			{
				--identStart;
			}
			while (not isalpha( ioDef.second[ identStart ]))
			{
				++identStart;
			}
			if (identStart == foundOff)
			{
				ioDef.second.insert( foundOff, 1, '_' );
				foundOff += 1;
			}
		}
		startOff = foundOff + 1;
	}
	ioDef.first.insert( 0, 1, '_' );
}

void	SCalcState::SetFunc( const std::string& inFuncName,
								const std::string& inFormalParam,
								const std::string& inValue )
{
	UnaryFuncDef	thePair( inFormalParam, inValue );
	FixFormalParam( thePair );
	UnaryFuncDef*	foundDef = find( mUnaryFuncDefs, inFuncName.c_str() );
	if (foundDef == NULL)
	{
		mUnaryFuncDefs.add( inFuncName.c_str(), thePair );
	}
	else
	{
		*foundDef = thePair;
	}
}

CFDictionaryRef		SCalcState::CopyVariables() const
{
	autoCFMutableDictionaryRef	theDict( ::CFDictionaryCreateMutable( NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
	ThrowIfCFFail_( theDict.get() );
	
	for (StringSet::iterator i = mVariableSet.begin(); i != mVariableSet.end(); ++i)
	{
		const std::string&	theVar( *i );
		double*	foundVal = find( mVariables, theVar.c_str() );
		if (foundVal != NULL)
		{
			autoCFStringRef	cfVarName( UTF8ToCFString( theVar.c_str() ) );
			ThrowIfCFFail_( cfVarName.get() );
			autoCFNumberRef	cfValue( ::CFNumberCreate( NULL, kCFNumberDoubleType,
				foundVal ) );
			ThrowIfCFFail_( cfValue.get() );
			::CFDictionaryAddValue( theDict.get(), cfVarName.get(), cfValue.get() );
		}
	}
	
	return theDict.release();
}

static void VarSetter( const void *key, const void *value, void *context )
{
	SCalcState*	me = static_cast<SCalcState*>( context );
	CFStringRef	theKey = static_cast<CFStringRef>( key );
	CFNumberRef	theValueRef = static_cast<CFNumberRef>( value );
	
	std::string	keyStr( CFStringToUTF8( theKey ) );
	double	theValue;
	::CFNumberGetValue( theValueRef, kCFNumberDoubleType, &theValue );
	
	double*	foundVar = find( me->mVariables, keyStr.c_str() );
	
	if (foundVar == NULL)
	{
		me->mVariables.add( keyStr.c_str(), theValue );
		me->mVariableSet.insert( keyStr );
	}
	else
	{
		*foundVar = theValue;
	}
}

void	SCalcState::SetVariables( CFDictionaryRef inDict )
{
	::CFDictionaryApplyFunction( inDict, VarSetter, this );
}

inline void DumpStack( const DblStack& inStack, const char* inTag )
{
	std::cout << inTag << ": ";
	for (DblStack::const_iterator i = inStack.begin(); i != inStack.end(); ++i)
	{
		std::cout << *i << ", ";
	}
	std::cout << std::endl;
}

namespace
{
	#pragma mark BinFunctor
	/*!
		@function	BinFunctor
		@abstract	Functor template that converts a binary function to a
					binary functor.
	*/
	template <BinaryFunc F>
	struct BinFunctor : std::binary_function< double, double, double >
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
	
	#pragma mark DoUnaryFuncDef
	struct DoUnaryFuncDef
	{
				DoUnaryFuncDef( SCalcState& ioState ) : mState( ioState ) {}
				DoUnaryFuncDef( const DoUnaryFuncDef& inOther ) : mState( inOther.mState ) {}
				
		void	operator()( const char* inStart, const char* inEnd ) const
				{
					std::string	parsedText( inStart, inEnd );
					std::string::size_type	parenLoc = parsedText.find( '(' );
					if (parenLoc == std::string::npos)
					{
						throw CalcException();
					}
					parsedText.erase( parenLoc );
					UnaryFuncDef*	foundFunc = find( mState.mUnaryFuncDefs, parsedText.c_str() );
					if (foundFunc == NULL)
					{
						throw CalcException();
					}
					SCalcState	tempState( mState );
					tempState.SetVariable( foundFunc->first.c_str() );
					double	funcVal;
					long	stopOff;
					ECalcResult	didParse = ParseCalcLine( foundFunc->second.c_str(), &tempState,
						&funcVal, &stopOff );
					if (didParse == kCalcResult_Calculated)
					{
						mState.mValStack.back() = funcVal;
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
				}
		
		SCalcState&		mState;
	};
	
	#pragma mark DoDefFunc
	struct DoDefFunc
	{
				DoDefFunc( SCalcState& ioState ) : mState( ioState ) {}
				DoDefFunc( const DoDefFunc& inOther ) : mState( inOther.mState ) {}
		
		void	operator()( const char*, const char* ) const
				{
					mState.SetFunc( mState.mFuncName, mState.mFuncParam, mState.mFuncDef );
					mState.mValStack.push_back(0.0);
					mState.mDidDefineFunction = true;
					//std::cout << "DoDefFunc " << mState.mFuncName << ", " << mState.mFuncParam <<
					//	", " << mState.mFuncDef << std::endl;
					//mState.SetVariable( mState.mIdentifier.c_str() );
					//mState.SetVariable( "last" );
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

	#pragma mark struct calculator
	struct calculator : public boost::spirit::grammar<calculator>
	{
					calculator( SCalcState& inState )
						: mState( inState ) {}

		template <typename ScannerT>
		struct definition
		{
			definition( const calculator& self )
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
					>> '(' >> identifier[ assign(self.mState.mFuncParam) ] >> ')'
					>> '=' >> lexeme_d[*anychar_p][ assign(self.mState.mFuncDef) ];
					
				statement = (
					assignment[ DoAssign(self.mState) ]
					|
					funcdef[ DoDefFunc(self.mState) ]
					|
					expression[ DoEvaluation(self.mState) ]
					)
					>> end_p;
				
				factor
					=	lexeme_d[ str_p("0x") >> hex_p[ append(self.mState.mValStack) ]]
					|	ureal_p[ append(self.mState.mValStack) ]
					|	self.mState.mVariables[ append(self.mState.mValStack) ]
					|	self.mState.mFixed.mConstants[ append(self.mState.mValStack) ]
					|	'(' >> expression >> ')'
					|	(lexeme_d[self.mState.mFixed.mUnaryFuncs >> '('] >> expression
							>> ')')[ DoUnaryFunc(self.mState) ]
					|	(lexeme_d[self.mState.mUnaryFuncDefs >> '('] >> expression
							>> ')')[ DoUnaryFuncDef(self.mState) ]
					|	(lexeme_d[self.mState.mFixed.mBinaryFuncs >> '('] >> expression
							>> ',' >> expression >> ')')[ DoBinaryFunc(self.mState) ]
					|	( "if(" >> expression >> ','
							>> expressionNA[ assign(self.mState.mIf1) ] >> ','
							>> expressionNA[ assign(self.mState.mIf2) ] >> ')'
						)[ DoIf( self.mState ) ]
					;
					// Note: The hex part of factor must come before the real part, otherwise ureal_p
					// will gobble the leading 0.
				
				factorNA
					=	lexeme_d[ str_p("0x") >> hex_p ]
					|	ureal_p
					|	self.mState.mVariables
					|	self.mState.mFixed.mConstants
					|	'(' >> expressionNA >> ')'
					|	(lexeme_d[self.mState.mFixed.mUnaryFuncs >> '('] >> expressionNA
							>> ')')
					|	(lexeme_d[self.mState.mUnaryFuncDefs >> '('] >> expressionNA
							>> ')')
					|	(lexeme_d[self.mState.mFixed.mBinaryFuncs >> '('] >> expressionNA
							>> ',' >> expressionNA >> ')')
					|	"if(" >> expressionNA >> ',' >> expressionNA >> ','
							>> expressionNA >> ')';
				
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
				return statement;
			}
			
			boost::spirit::rule<ScannerT>	factor, expression, term, power, identifier;
			boost::spirit::rule<ScannerT>	factorNA, expressionNA, termNA, powerNA;
			boost::spirit::rule<ScannerT>	assignment, statement, funcdef;
		};
		
		SCalcState&	mState;
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
	@function	ParseCalcLine
	@abstract	Attempt to parse and compute an expression or assignment.
	@param		inLine		A NUL-terminated line of text.
	@param		ioState		A calculator object reference.
	@param		outValue	Receives the computed value if the computation
							succeeded.
	@param		outStop		Offset at which parsing stopped, which can be
							helpful in spotting a syntax error.
	@result		Whether we failed, calculated, or defined a function.
*/
ECalcResult		ParseCalcLine( const char* inLine, CalcState ioState, double* outValue,
						long* outStop )
{
	ECalcResult	didParse = kCalcResult_Error;
	
	try
	{
		calculator	theCalc( *ioState );
		ioState->mValStack.clear();
		
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

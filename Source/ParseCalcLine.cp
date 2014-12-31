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
//#include "boost/spirit.hpp"
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
		( "%", 0.01 )
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
									const StringVec& inFormalParams,
									const std::string& inValue );
	
	CFDictionaryRef			CopyVariables() const;
	void					SetVariables( CFDictionaryRef inDict );
	
	CFDictionaryRef			CopyFuncDefs() const;
	void					SetFuncDefs( CFDictionaryRef inDict );
	
	const SFixedSymbols&	mFixed;
	
	DblStack				mValStack;
	StringVec				mParamStack;
	symbols<FuncDef>		mFuncDefs;
	symbols<double>			mVariables;
	std::string				mIdentifier;
	StringSet				mVariableSet;
	StringSet				mFuncDefSet;
	std::string				mFuncName;
	std::string				mFuncParam;
	std::string				mFuncDef;
	std::string				mIf1;
	std::string				mIf2;
	std::string				mDefinedSymbol;
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
	, mParamStack( inOther.mParamStack )
	, mFuncDefs( inOther.mFuncDefs )
	, mVariables( inOther.mVariables )
	, mIdentifier( inOther.mIdentifier )
	, mVariableSet( inOther.mVariableSet )
	, mFuncDefSet( inOther.mFuncDefSet )
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
	@function	FixFormalParams
	@abstract	Prefix each formal parameter by an @, so it cannot be
				confused with a normal variable.
*/
static void FixFormalParams( FuncDef& ioDef )
{
	std::string&	rightHandSide( ioDef.second );
	
	for (StringVec::iterator i = ioDef.first.begin(); i != ioDef.first.end(); ++i)
	{
		std::string&	theParam( *i );
		
		std::string::size_type	startOff = 0;
		std::string::size_type	foundOff, identStart, identEnd;
		
		while ( (foundOff = rightHandSide.find( theParam, startOff )) !=
			std::string::npos )
		{
			// We found a substring that looks like the formal parameter, but we
			// must be sure that it is not just part of some other identifier.
			identStart = foundOff;
			identEnd = identStart + theParam.size();
			if ( (identEnd == rightHandSide.size()) or
				(not IsIdentifierChar(rightHandSide[identEnd])) )
			{
				// Looking to the left is trickier, e.g., while 12x is not an
				// identifier, A12x is.
				while ( (identStart > 0) and
					IsIdentifierChar(rightHandSide[ identStart - 1 ]) )
				{
					--identStart;
				}
				while (not isalpha( rightHandSide[ identStart ]))
				{
					++identStart;
				}
				if (identStart == foundOff)
				{
					rightHandSide.insert( foundOff, 1, '@' );
					foundOff += 1;
				}
			}
			startOff = foundOff + 1;
		}
		
		theParam.insert( 0, 1, '@' );
	}
}


void	SCalcState::SetFunc( const std::string& inFuncName,
								const StringVec& inFormalParams,
								const std::string& inValue )
{
	FuncDef	thePair( inFormalParams, inValue );
	FixFormalParams( thePair );
	FuncDef*	foundDef = find( mFuncDefs, inFuncName.c_str() );
	if (foundDef == NULL)
	{
		mFuncDefs.add( inFuncName.c_str(), thePair );
		mFuncDefSet.insert( inFuncName );
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

/*!
	@function	UnfixParams
	@abstract	Remove @ characters from names
*/
static void UnfixParams( std::string& ioParam )
{
	std::string::size_type	atLoc;
	
	while ((atLoc = ioParam.find('@')) != std::string::npos)
	{
		ioParam.erase( atLoc, 1 );
	}
}

CFDictionaryRef	SCalcState::CopyFuncDefs() const
{
	autoCFMutableDictionaryRef	theDict( ::CFDictionaryCreateMutable( NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
	ThrowIfCFFail_( theDict.get() );
	
	for (StringSet::iterator i = mFuncDefSet.begin(); i != mFuncDefSet.end(); ++i)
	{
		// Find the function definition.
		const std::string&	funcName( *i );
		FuncDef*	theFuncPtr = find( mFuncDefs, funcName.c_str() );
		if (theFuncPtr != NULL)
		{
			// Make a copy so we can change it
			FuncDef	theFunc( *theFuncPtr );
		
			// Get name as CFString
			autoCFStringRef	cfFuncName( UTF8ToCFString( funcName.c_str() ) );
			ThrowIfCFFail_( cfFuncName.get() );
			
			// Create an emtpy array
			autoCFMutableArrayRef	theArray( ::CFArrayCreateMutable(
				NULL, 0, &kCFTypeArrayCallBacks ) );
			ThrowIfCFFail_( theArray.get() );

			// Put the RHS in the array
			UnfixParams( theFunc.second );
			autoCFStringRef	cfFuncDef( UTF8ToCFString( theFunc.second.c_str() ) );
			ThrowIfCFFail_( cfFuncDef.get() );
			::CFArrayAppendValue( theArray.get(), cfFuncDef.get() );
			
			for (StringVec::iterator i = theFunc.first.begin();
				i != theFunc.first.end(); ++i)
			{
				UnfixParams( *i );
				autoCFStringRef	cfParam( UTF8ToCFString( i->c_str() ) );
				ThrowIfCFFail_( cfParam.get() );
				::CFArrayAppendValue( theArray.get(), cfParam.get() );
			}
			
			::CFDictionarySetValue( theDict.get(), cfFuncName.get(),
				theArray.get() );
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
	
	me->mValStack.push_back( theValue );
	me->SetVariable( keyStr.c_str() );
	me->mValStack.pop_back();
}

void	SCalcState::SetVariables( CFDictionaryRef inDict )
{
	::CFDictionaryApplyFunction( inDict, VarSetter, this );
}

static void FuncSetter( const void *key, const void *value, void *context )
{
	SCalcState*	me = static_cast<SCalcState*>( context );
	CFStringRef	theKey = static_cast<CFStringRef>( key );
	CFArrayRef	theValueRef = static_cast<CFArrayRef>( value );
	std::string	keyStr( CFStringToUTF8( theKey ) );
	CFIndex	arraySize = ::CFArrayGetCount( theValueRef );
	CFStringRef	funcDefRef = reinterpret_cast<CFStringRef>(
		::CFArrayGetValueAtIndex( theValueRef, 0 ) );
	FuncDef	theFunc;
	theFunc.second = CFStringToUTF8( funcDefRef );
	for (int i = 1; i < arraySize; ++i)
	{
		std::string	param( CFStringToUTF8( reinterpret_cast<CFStringRef>(
			::CFArrayGetValueAtIndex( theValueRef, i ) ) ) );
		theFunc.first.push_back( param );
	}
	
	me->SetFunc( keyStr, theFunc.first, theFunc.second );
}

void	SCalcState::SetFuncDefs( CFDictionaryRef inDict )
{
	::CFDictionaryApplyFunction( inDict, FuncSetter, this );
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

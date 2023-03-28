//  MiniCalc.cpp
//  MiniCalcTests
//
//  Created by James Walker on 3/27/23.
//  
//

#include "MiniCalc.hpp"

#include "SCalcState.hpp"

#include <math.h>

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 1

#pragma warn_unusedarg	off
#if !defined(BOOST_SPIRIT_USE_OLD_NAMESPACE)
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#endif
#include "boost/spirit/include/classic.hpp"
#pragma warn_unusedarg	reset

using namespace boost::spirit;

namespace
{
struct DoPlus
{
			DoPlus( SCalcState& ioState ) : mState( ioState ) {}
			DoPlus( const DoPlus& inOther ) : mState( inOther.mState ) {}
	
	void	operator()( const char*, const char* ) const
			{
				double	rhs = mState.mValStack.back();
				mState.mValStack.pop_back();
				double	lhs = mState.mValStack.back();
				mState.mValStack.pop_back();
				double	theVal = lhs + rhs;
				mState.mValStack.push_back( theVal );
			}
	
	SCalcState&		mState;
};

/*!
	@struct		DoEvaluation
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

	struct calculator : public boost::spirit::grammar<calculator>
	{
					calculator( SCalcState& inState )
						: mState( inState ) {}

		template <typename ScannerT>
		struct definition
		{
			definition( const calculator& self )
			{
				statement =
					expression[ DoEvaluation(self.mState) ]
					>> end_p;
				
				term
					=	ureal_p[ append(self.mState.mValStack) ]
					;
				
				expression =
						term
						>>
						*(
							('+' >> term)[ DoPlus(self.mState) ]
						);
			}
			
			const boost::spirit::rule<ScannerT>& start() const
			{
				return statement;
			}
			
			boost::spirit::rule<ScannerT>	term, expression;
			boost::spirit::rule<ScannerT>	statement;
		};
		
		SCalcState&		mState;
	};
}

/*!
	@function	MiniCalc
	
	@abstract	Do a calculation similar to what ParseCalcLine would do, but with a minimal grammar
				that is only able to add.
	
	@param		inLine		A NUL-terminated line of text.
	@param		ioState		A calculator object reference.
	@result		The calculated result, or NaN on failure.
*/
double MiniCalc( const char* inLine, struct SCalcState* ioState )
{
	double result = NAN;
	
	try
	{
		calculator	theCalc( *ioState );
		ioState->mValStack.clear();
		ioState->mParamStack.clear();
		
		parse_info<>	parseResult = parse( inLine, theCalc, space_p );
		
		if (parseResult.full)
		{
			if (ioState->mValStack.size() >= 1)
			{
				result = ioState->mValStack.back();
			}
		}
	}
	catch (...)
	{
	}
	
	return result;
}

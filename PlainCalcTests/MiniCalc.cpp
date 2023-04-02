//  MiniCalc.cpp
//  MiniCalcTests
//
//  Created by James Walker on 3/27/23.
//  
//

#include "MiniCalc.hpp"

#include "SCalcState.hpp"

#include <math.h>

using namespace boost::spirit::qi;

namespace
{
struct DoPlus
{
			DoPlus( SCalcState& ioState ) : mState( ioState ) {}
			DoPlus( const DoPlus& inOther ) : mState( inOther.mState ) {}
	
	void	operator()( unused_type, unused_type, unused_type ) const
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
	
	void	operator()( unused_type, unused_type, unused_type ) const
	{
		mState.SetVariable( "last" );
		mState.mDidDefineFunction = false;
		mState.mDefinedSymbol.clear();
	}
	
	SCalcState&		mState;
};

struct DoAppendNumber
{
			DoAppendNumber( SCalcState& ioState ) : mState( ioState ) {}
			DoAppendNumber( const DoAppendNumber& inOther ) : mState( inOther.mState ) {}
	
	void	operator()( double val, unused_type, unused_type ) const
	{
		mState.mValStack.push_back( val );
	}
	
	SCalcState&		mState;
};

	template <typename Iterator>
	struct calculator : public grammar<Iterator, double(), ascii::space_type>
	{
		calculator( SCalcState& inState )
			: calculator::base_type( start )
			, mState( inState )
		{
			statement =
				expression//[ DoEvaluation(mState) ]
				>> eoi;
			
			term
				=	ureal//[ DoAppendNumber(mState) ]
				;
			
			expression =
					term
					>>
					*(
						('^' >> term)//[ DoPlus(mState) ]
					);
		
			start =
				statement;
		}
		
			
		real_parser< double, ureal_policies<double> > ureal;
		
		rule<Iterator, double(), ascii::space_type>	term, expression;
		rule<Iterator, double(), ascii::space_type>	statement;
		rule<Iterator, double(), ascii::space_type>	start;

		SCalcState&		mState;
	};

	template <typename Iterator>
	struct syntaxCheck : public grammar<Iterator, ascii::space_type>
	{
		syntaxCheck()
			: syntaxCheck::base_type( statement )
		{
			statement =
				factor
				>> eoi;
			
			factor =
				double_
				>>
				-('^' >> double_)
				;
		}
		
		rule<Iterator, ascii::space_type>	factor;
		rule<Iterator, ascii::space_type>	statement;
	};
}

/*!
	@function	CheckExpressionSyntax
	@abstract	Check the syntax of an expression.
	@param		inLine		A NUL-terminated line of text.
	@result		True if the expression was parsed successfully.
*/
bool	CheckExpressionSyntax( const char* inLine )
{
	bool	isOK = false;
	
	try
	{
		syntaxCheck<const char*>	theCalc;
		
		const char* startIter = inLine;
		const char* endIter = inLine + strlen(inLine);

		bool success = phrase_parse( startIter, endIter, theCalc, ascii::space );
		
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
		ioState->mValStack.clear();
		ioState->mParamStack.clear();

		const char* startIter = inLine;
		const char* endIter = inLine + strlen(inLine);

		calculator<const char*>	theCalc( *ioState );
		
		bool success = phrase_parse( startIter, endIter, theCalc, ascii::space );
		
		if (success and (startIter == endIter))
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

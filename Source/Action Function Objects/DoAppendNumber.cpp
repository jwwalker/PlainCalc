//  DoAppendNumber.cpp
//  PlainCalc2
//
//  Created by James Walker on 3/28/23.
//  
//

#include "DoAppendNumber.hpp"

#include "SCalcState.hpp"

void	DoAppendNumber::operator()( double val ) const
{
	mState.mValStack.push_back( val );
}

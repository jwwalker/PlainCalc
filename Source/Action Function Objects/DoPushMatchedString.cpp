//  DoPushMatchedString.cpp
//  PlainCalc2
//
//  Created by James Walker on 3/30/23.
//  
//

#include "DoPushMatchedString.hpp"

#include "SCalcState.hpp"

void DoPushMatchedString::operator()( boost::iterator_range<const char*>& matchRange ) const
{
	std::string theMatch( matchRange.begin(), matchRange.end() );
	mState.mParamStack.push_back( std::move(theMatch) );
}

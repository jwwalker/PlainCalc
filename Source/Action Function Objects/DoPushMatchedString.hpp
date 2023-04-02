//  DoPushMatchedString.hpp
//  PlainCalc2
//
//  Created by James Walker on 3/30/23.
//  
//

#ifndef DoPushMatchedString_hpp
#define DoPushMatchedString_hpp

struct SCalcState;

#include <boost/range.hpp>


/*!
	@struct		DoPushMatchedString
	
	@abstract	Push the matched string of a parser encosed in the raw[] directive onto the
				vector mParamStack in SCalcState.
*/
struct DoPushMatchedString
{
			DoPushMatchedString( SCalcState& ioState ) : mState( ioState ) {}
			DoPushMatchedString( const DoPushMatchedString& inOther )
				: mState( inOther.mState ) {}
	
	void operator()( boost::iterator_range<const char*>& matchRange ) const;
	
	
	SCalcState&		mState;
};

#endif /* DoPushMatchedString_hpp */

//  ConvertErrorOffset.hpp
//  PlainCalc3
//
//  Created by James Walker on 9/30/25.
//  
//

#ifndef ConvertErrorOffset_hpp
#define ConvertErrorOffset_hpp

#import <CoreFoundation/CoreFoundation.h>


/*!
	@function	ConvertErrorOffset
	
	@abstract	Convert an error offset from a number of  UTF-32 code points to a number of
				UTF-16 code units.
	
	@param		inCalculatedLine	Line of text that was calculated.
	@param		inUTF32ErrorOffset	Offset of error location in UTF-32 text.
	@result		Offset of error location in UTF-32 code points.
*/
CFIndex	ConvertErrorOffset( CFStringRef inCalculatedLine,
						CFIndex inUTF32ErrorOffset );

#endif /* ConvertErrorOffset_hpp */

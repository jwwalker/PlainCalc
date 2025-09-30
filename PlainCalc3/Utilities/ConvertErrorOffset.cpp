//  ConvertErrorOffset.cpp
//  PlainCalc3
//
//  Created by James Walker on 9/30/25.
//  
//

#import "ConvertErrorOffset.hpp"

#import <vector>
#import <inttypes.h>

/*!
	@function	ConvertErrorOffset
	
	@abstract	Convert an error offset from a number of  UTF-32 code points to a number of
				UTF-16 code units.
	
	@param		inCalculatedLine	Line of text that was calculated.
	@param		inUTF32ErrorOffset	Offset of error location in UTF-32 text.
	@result		Offset of error location in UTF-32 code points.
*/
CFIndex	ConvertErrorOffset( CFStringRef inCalculatedLine,
						CFIndex inUTF32ErrorOffset )
{
	CFIndex offset16 = 0;
	
	CFIndex origLen = CFStringGetLength( inCalculatedLine );
	std::vector<uint32_t>	workBuf( origLen );
	CFIndex bytesUsed = 0;
	CFStringGetBytes( inCalculatedLine, CFRangeMake( 0, origLen ),
		kCFStringEncodingUTF32LE, '?', false,
		reinterpret_cast<uint8_t*>( workBuf.data() ),
		workBuf.size() * sizeof(uint32_t), &bytesUsed );
	workBuf.resize( bytesUsed / sizeof(uint32_t) );
	
	// Truncate our UTF-32 string down to the part up to the error offset
	if ( (inUTF32ErrorOffset >= 0) and (inUTF32ErrorOffset < workBuf.size()) )
	{
		workBuf.resize( inUTF32ErrorOffset  );
	}
	
	// Convert back to UTF-16
	CFStringRef upToError = CFStringCreateWithBytes( nullptr,
		reinterpret_cast<const uint8_t*>( workBuf.data() ),
		workBuf.size() * sizeof(uint32_t), kCFStringEncodingUTF32LE, false );
	if (upToError != nullptr)
	{
		offset16 = CFStringGetLength( upToError );
		CFRelease( upToError );
	}
	
	return offset16;
}

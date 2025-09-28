//  UTF32toUTF8.cpp
//  ParserPlay
//
//  Created by James Walker on 8/8/25.
//  
//
/*
	Copyright (c) 2025 James W. Walker

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

#import "UTF32toUTF8.hpp"

#import <CoreFoundation/CoreFoundation.h>
#import <algorithm>

static constexpr size_t maxUTF8CharsPerUTF32Code = 4;

std::string UTF32toUTF8( const std::u32string& inStr32 )
{
	std::string result;

	// If every code point is less than or equal to 0x7F, then we can do it
	// the easy way.
	if (std::all_of( inStr32.cbegin(), inStr32.cend(),
		[](char32_t codept){ return codept <= 0x7F; } ))
	{
		for (char32_t codept : inStr32)
		{
			result += static_cast<char>( codept );
		}
	}
	else
	{
		CFStringRef strCF = CFStringCreateWithBytes( nullptr,
			(const UInt8*) inStr32.data(), inStr32.size() * sizeof(char32_t),
			kCFStringEncodingUTF32LE, false );
		
		if (strCF != nullptr)
		{
			result.resize( inStr32.size() * maxUTF8CharsPerUTF32Code );

			CFIndex bytesInBuffer = 0;
			CFStringGetBytes( strCF, CFRangeMake( 0, CFStringGetLength(strCF) ),
				kCFStringEncodingUTF8, '?', false, (UInt8*) result.data(),
				result.size(), &bytesInBuffer );
				
			result.resize( bytesInBuffer );
			
			CFRelease( strCF );
		}
	}
	
	return result;
}

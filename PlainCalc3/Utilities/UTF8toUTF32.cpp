//  UTF8toUTF32.cpp
//  ParserPlay
//
//  Created by James Walker on 8/16/25.
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

#import "UTF8toUTF32.hpp"

#import <CoreFoundation/CoreFoundation.h>
#import <algorithm>

std::u32string UTF8toUTF32( const std::string& inStr )
{
	std::u32string result;

	// If every code point is less than or equal to 0x7F, then we can do it
	// the easy way.
	if (std::all_of( inStr.cbegin(), inStr.cend(),
		[](char codept){ return static_cast<unsigned char>(codept) <= 0x7F; } ))
	{
		result.reserve( inStr.size() );
		for (char codept : inStr)
		{
			result += static_cast<char32_t>( codept );
		}
	}
	else
	{
		CFStringRef strCF = CFStringCreateWithCString( nullptr, inStr.c_str(),
			kCFStringEncodingUTF8 );
		if (strCF != nullptr)
		{
			result.resize( inStr.size() );
			auto fullRange = CFRangeMake( 0, CFStringGetLength(strCF) );
			CFIndex bytesInBuffer = 0;
			CFStringGetBytes( strCF, fullRange,
				kCFStringEncodingUTF32LE, '?', false, (UInt8*) result.data(),
				result.size() * sizeof(char32_t), &bytesInBuffer );

			// Probably bytesInBuffer is a multiple of 4, but just to be safe...
			size_t codeCount = (bytesInBuffer + sizeof(char32_t) - 1) /
				sizeof(char32_t);
			result.resize( codeCount );
			
			CFRelease( strCF );
		}
	}
		
	return result;
}

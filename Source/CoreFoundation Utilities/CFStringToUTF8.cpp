#include "CFStringToUTF8.h"

/*
	Copyright (c) 2006-2015 James W. Walker

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

#include <string>
#include <vector>


std::string		CFStringToUTF8( CFStringRef inStr, CFStringEncoding inEncoding )
{
	std::string	resultStr;
	
	if (inStr != NULL)
	{
		CFIndex		bufLen = ::CFStringGetMaximumSizeForEncoding(
			::CFStringGetLength( inStr ), inEncoding );
		
		std::vector<char>	buffer( bufLen + 1 );
		if (::CFStringGetCString(inStr, &buffer.front(), buffer.size(),
			inEncoding ))
		{
			resultStr.assign( &buffer.front() );
		}
	}
	
	return resultStr;
}


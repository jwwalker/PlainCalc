#include "CFStringToUTF8.h"

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


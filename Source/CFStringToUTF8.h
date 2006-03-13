#pragma once

#if __MACH__
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <CFBase.h>
#endif

#include <string>

std::string		CFStringToUTF8( CFStringRef inStr,
	CFStringEncoding inEncoding = kCFStringEncodingUTF8 );

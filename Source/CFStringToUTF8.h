#pragma once

#include <CFBase.h>
#include <stringfwd>

std::string		CFStringToUTF8( CFStringRef inStr,
	CFStringEncoding inEncoding = kCFStringEncodingUTF8 );

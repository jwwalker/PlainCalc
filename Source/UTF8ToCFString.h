#pragma once

#include "autoCF.h"

/*!
	@function	UTF8ToCFString
	
	@abstract	Create a CFStringRef from a C string in UTF-8 encoding.
*/
autoCFStringRef		UTF8ToCFString( const char* inStr );


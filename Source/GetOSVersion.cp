#include "GetOSVersion.h"

/*!
	@function	GetOSVersion
	
	@abstract	Get the OS version.
	
	@discussion	The version is returned in binary-coded decimal form.
				For example, OS 10.2.6 would be represented as 0x1026.
	
	@result		The system version.
*/
SInt32	GetOSVersion()
{
	static SInt32	sVers = 0;
	
	if (sVers == 0)
	{
		::Gestalt( gestaltSystemVersion, &sVers );
	}
	
	return sVers;
}

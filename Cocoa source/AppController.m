//
//  AppController.m
//  PlainCalc2
//
//  Created by James Walker on 12/31/09.

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

#import "AppController.h"

#import <math.h>

static AppController* sMe;

@implementation AppController


+ (NSColor*) defaultGreen
{
	return [NSColor colorNamed: @"GoodResult" ];
}


+ (void) initialize
{
}


- (id) init
{
	self = [super init];
	
	if ( self != nil )
	{
		sMe = self;
		
		mNormalColorAtt = @{
			NSForegroundColorAttributeName: [NSColor textColor]
		};
		mErrorColorAtt = @{
			NSForegroundColorAttributeName: [NSColor colorNamed: @"BadResult"]
		};
		mCalcTimeout = 1.0;
		
		NSString* decimalSep = [NSLocale.currentLocale objectForKey:
			NSLocaleDecimalSeparator ];
		mCommaIsDecimal = [decimalSep isEqualToString: @","];

		[NSNotificationCenter.defaultCenter
			addObserver: self
			selector: @selector(localeChanged:)
			name: NSCurrentLocaleDidChangeNotification
			object: nil];
	}
	return self;
}

+ (AppController*) sharedController
{
	return sMe;
}

- (void) localeChanged: (NSNotification*) notification
{
	NSString* decimalSep = [NSLocale.currentLocale objectForKey:
		NSLocaleDecimalSeparator ];
	mCommaIsDecimal = [decimalSep isEqualToString: @","];
	//NSLog(@"Decimal separator is %@", mCommaIsDecimal? @"comma" : @"period");
}

+ (NSDictionary*) normalAtts
{
	return sMe->mNormalColorAtt;
}

+ (NSDictionary*) errorAtts
{
	return sMe->mErrorColorAtt;
}

+ (NSDictionary*) successAtts
{
	NSColor* goodColor = [AppController defaultGreen];
	return @{
		NSForegroundColorAttributeName: goodColor
	};
}

+ (double) calcTimeout
{
	return sMe->mCalcTimeout;
}

+ (BOOL) isCommaDecimal
{
	return sMe->mCommaIsDecimal;
}

@end

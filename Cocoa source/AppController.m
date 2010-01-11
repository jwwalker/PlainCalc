//
//  AppController.m
//  PlainCalc2
//
//  Created by James Walker on 12/31/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import "AppController.h"

#import "NSUserDefaults_JWColorSupport.h"
#import "ExponentialTransformer.h"

#import <math.h>

static AppController* sMe;

@implementation AppController

- (NSDictionary*) colorAttsforPref: (NSString*) prefKey
					withDefault: (NSColor*) defColor
{
	NSColor* theColor = [[NSUserDefaults standardUserDefaults]
		jw_colorForKey: prefKey];
	if (theColor == nil)
	{
		theColor = defColor;
	}
	
	NSDictionary* theDict = [NSDictionary dictionaryWithObjectsAndKeys:
		theColor, NSForegroundColorAttributeName, nil ];
	
	return theDict;
}

+ (NSColor*) defaultGreen
{
	return [NSColor colorWithDeviceRed: 0.0
					green: 0.6
					blue: 0.0
					alpha: 1.0];
}

+ (void) setDefaultPrefs
{
	NSDictionary* defPrefs = [NSDictionary dictionaryWithObjectsAndKeys:
		[NSArchiver archivedDataWithRootObject: [NSColor blackColor]],
		@"NormalColor",
		[NSArchiver archivedDataWithRootObject: [NSColor redColor]],
		@"ErrorColor",
		[NSArchiver archivedDataWithRootObject: [self defaultGreen]],
		@"SuccessColor",
		[NSNumber numberWithFloat: 0.0f],
		@"TimeoutExponent",
		nil];
	
	[[NSUserDefaults standardUserDefaults] registerDefaults: defPrefs];
}

+ (void) initialize
{
	[AppController setDefaultPrefs];
	
	ExponentialTransformer* powerOf2 = [[[ExponentialTransformer alloc]
		initWithBase: 2.0] autorelease];
	[NSValueTransformer setValueTransformer: powerOf2
						forName: @"PowerOf2" ];
}

- (double) prefTimeout
{
	double theExp = [[NSUserDefaults standardUserDefaults] floatForKey:
			@"TimeoutExponent" ];
	double theTime = pow( 2.0, theExp );
	return theTime;
}

- (id) init
{
	if ( (self = [super init]) != nil )
	{
		sMe = self;
		
		mNormalColorAtt = [[self colorAttsforPref: @"NormalColor"
								withDefault: [NSColor blackColor] ] retain];
		mErrorColorAtt = [[self colorAttsforPref: @"ErrorColor"
								withDefault: [NSColor redColor] ] retain];
		mSuccessColorAtt = [[self colorAttsforPref: @"SuccessColor"
								withDefault: [AppController defaultGreen] ] retain];
		mCalcTimeout = [self prefTimeout];
		
		NSString* decimalSep = [[NSLocale currentLocale] objectForKey:
			NSLocaleDecimalSeparator ];
		mCommaIsDecimal = [decimalSep isEqualToString: @","];

		[[NSUserDefaults standardUserDefaults]
			addObserver: self
			forKeyPath: @"ErrorColor"
			options: 0
			context: NULL];
		[[NSUserDefaults standardUserDefaults]
			addObserver: self
			forKeyPath: @"SuccessColor"
			options: 0
			context: NULL];
		[[NSUserDefaults standardUserDefaults]
			addObserver: self
			forKeyPath: @"TimeoutExponent"
			options: 0
			context: NULL];
	}
	return self;
}

+ (AppController*) sharedController
{
	return sMe;
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
	return sMe->mSuccessColorAtt;
}

+ (double) calcTimeout
{
	return sMe->mCalcTimeout;
}

+ (BOOL) isCommaDecimal
{
	return sMe->mCommaIsDecimal;
}

- (void) observeValueForKeyPath:(NSString *)keyPath
		ofObject:(id)object
		change:(NSDictionary *)change
		context:(void *)context
{
	if ([keyPath isEqualToString: @"SuccessColor"])
	{
		[mSuccessColorAtt release];
		mSuccessColorAtt = [[self colorAttsforPref: keyPath
								withDefault: [AppController defaultGreen] ] retain];
	}
	else if ([keyPath isEqualToString: @"ErrorColor"])
	{
		[mErrorColorAtt release];
		mErrorColorAtt = [[self colorAttsforPref: keyPath
								withDefault: [NSColor redColor] ] retain];
	}
	else if ([keyPath isEqualToString: @"TimeoutExponent"])
	{
		mCalcTimeout = [self prefTimeout];
	}
}

@end

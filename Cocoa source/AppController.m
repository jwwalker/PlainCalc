//
//  AppController.m
//  PlainCalc2
//
//  Created by James Walker on 12/31/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import "AppController.h"

#import "NSUserDefaults_JWColorSupport.h"

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
		nil];
	
	[[NSUserDefaults standardUserDefaults] registerDefaults: defPrefs];
}

+ (void) initialize
{
	[AppController setDefaultPrefs];
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
}

@end

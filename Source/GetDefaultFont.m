//
//  GetDefaultFont.m
//  PlainCalc2
//
//  Created by James Walker on 12/13/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import "GetDefaultFont.h"

NSFont*	GetDefaultFont( void )
{
	NSFont* myFont = nil;
	NSUserDefaults*	defaults = [NSUserDefaults standardUserDefaults];
	
	NSString* fontFamily = [defaults stringForKey: @"DefaultFontName"];
	float fontSize = [defaults floatForKey: @"DefaultFontSize"];
	
	if ( (fontFamily != nil) && (fontSize > 0.0f) )
	{
		NSFontManager*	fontMgr = [NSFontManager sharedFontManager];
		myFont = [fontMgr
				fontWithFamily: fontFamily
				traits: 0
				weight: 5
				size: fontSize];
	}
	
	if (myFont == nil)
	{
		myFont = [NSFont userFontOfSize: 14];
	}
	
	return myFont;
}

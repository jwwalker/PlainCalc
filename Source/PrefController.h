//
//  PrefController.h
//  PlainCalc2
//
//  Created by James Walker on 12/13/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class FontResponder;

@interface PrefController : NSObject
{
	IBOutlet NSTextField*	oFontSample;
	
	NSFont*					mDefaultFont;
	NSFontPanel*			mFontPanel;
	FontResponder*			mFontResponder;
}

- (IBAction) doSelectFont: (id) sender;

@end

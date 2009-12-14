//
//  PrefController.m
//  PlainCalc2
//
//  Created by James Walker on 12/13/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import "PrefController.h"

#import "GetDefaultFont.h"

@interface PrefController ()

- (NSFont*) font;
- (void) setFont: (NSFont*) font;

@end

@interface FontResponder : NSResponder
{
	PrefController*	mController;
}

- (id) init: (PrefController*) controller;

- (void) changeFont: (id)sender;

@end

@implementation FontResponder

- (id) init: (PrefController*) controller
{
	if ( (self = [super init]) != nil )
	{
		mController = controller;
	}
	return self;
}

- (void) changeFont: (id)sender
{
	NSFont*	oldFont = [mController font];
	NSFont* newFont = [sender convertFont: oldFont];
	[mController setFont: newFont];
	
	NSUserDefaults*	defaults = [NSUserDefaults standardUserDefaults];
	[defaults setValue: [newFont familyName] forKey: @"DefaultFontName" ];
	[defaults setFloat: [newFont pointSize] forKey: @"DefaultFontSize" ];
}

@end


@implementation PrefController

- (id) init
{
	if ( (self = [super init]) != nil )
	{
		mFontResponder = [[FontResponder alloc] init: self];
	}
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver: self ];
	
	[mDefaultFont release];
	[mFontResponder release];
	
	[mFontPanel orderOut: nil];
	
	[super dealloc];
}

- (NSFont*) font
{
	return mDefaultFont;
}


- (void) updateFontSample
{
	NSString*	famAndSize = [NSString stringWithFormat: @"%@ - %.1f",
		[mDefaultFont familyName], [mDefaultFont pointSize] ];
	[oFontSample setStringValue: famAndSize];
	[oFontSample setFont: mDefaultFont ];
}

- (void) setFont: (NSFont*) font
{
	if (font != mDefaultFont)
	{
		[mDefaultFont release];
		mDefaultFont = [font retain];
		
		[self updateFontSample];
	}
}


- (void) initFont
{
	NSFont* myFont = GetDefaultFont();
	
	[self setFont: myFont];
	[[NSFontManager sharedFontManager] setSelectedFont: myFont isMultiple: NO ];
}

- (void) awakeFromNib
{
	[self initFont];
}


- (IBAction) doSelectFont: (id) sender
{
	NSFontManager*	fontMgr = [NSFontManager sharedFontManager];
	[fontMgr setSelectedFont: [self font] isMultiple: NO ];
	
	mFontPanel = [fontMgr fontPanel: YES];
	
	[mFontPanel makeKeyAndOrderFront: self];	
	
	[[oFontSample window] makeFirstResponder: mFontResponder];
}

@end

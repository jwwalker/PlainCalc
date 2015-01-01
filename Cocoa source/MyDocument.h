//
//  MyDocument.h
//  PlainCalc2
//
//  Created by James Walker on 3/11/06.

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


#import <Cocoa/Cocoa.h>

@interface MyDocument : NSDocument
{
	IBOutlet NSTextView *textView;
	IBOutlet NSWindow* docWindow;
	
	IBOutlet NSWindow*		oForgetSymbolSheet;
	IBOutlet NSPopUpButton*	oForgetSymbolPopup;
	
	NSAttributedString *mString;
	NSFont*			mInitialTypingFont;
	struct SCalcState* mCalcState;
	NSString*		mLoadedWindowFrame;
	BOOL mFormatIntegersAsHex;
	NSMenuItem*		mDecFormatItem;
	NSMenuItem*		mHexFormatItem;
	BOOL			mForgettingFunction;
	NSTask*			mCalcTask;
	NSMutableData*	mResultBuffer;
	NSMutableString*	mLineToCalculate;
	
	// Remark: I was wondering why we would need mString as well as textView,
	// since textView has a string as storage.   The reason seems to be that the
	// data arrives, via loadDataRepresentation, before
	// windowControllerDidLoadNib, so we may not have access to textView at that
	// point.  Perhaps there was also a lesser reason of illustrating the
	// model-view-controller pattern, but I will not bother to set the string
	// after the text has been loaded.
}

- (IBAction) showDefinedVariables: (id) sender;
- (IBAction) showDefinedFunctions: (id) sender;
- (IBAction) setIntegerFormat: (id) sender;
- (IBAction) pasteCleaned: (id) sender;
- (IBAction) saveAsNewDocumentContent: (id) sender;
- (IBAction) forgetFunction: (id) sender;
- (IBAction) forgetVariable: (id) sender;
- (IBAction) forgetSheetOK: (id) sender;
- (IBAction) forgetSheetCancel: (id) sender;



@end

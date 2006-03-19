//
//  MyDocument.h
//  PlainCalc2
//
//  Created by James Walker on 3/11/06.
//  Copyright James W. Walker 2006 . All rights reserved.
//


#import <Cocoa/Cocoa.h>

@interface MyDocument : NSDocument
{
	IBOutlet NSTextView *textView;
	NSAttributedString *mString;
	struct SCalcState* mCalcState;
	BOOL mFormatIntegersAsHex;
	NSMenuItem* mCurFormatItem;
	NSDictionary*	mErrorColorAtt;
	NSDictionary*	mSuccessColorAtt;
	NSDictionary*	mNormalColorAtt;
	
	// Remark: I was wondering why we would need mString as well as textView,
	// since textView has a string as storage.   The reason seems to be that the
	// data arrives, via loadDataRepresentation, before
	// windowControllerDidLoadNib, so we may not have access to textView at that
	// point.  Perhaps there was also a lesser reason of illustrating the
	// model-view-controller pattern, but I will not bother to set the string
	// after the text has been loaded.
}

- (void) setIntegerFormat: (id) sender;

- (void) setString: (NSAttributedString *) value;

@end
//
//  MyDocument.m
//  PlainCalc2
//
//  Created by James Walker on 3/11/06.
//  Copyright James W. Walker 2006 . All rights reserved.
//

#import "MyDocument.h"
#import "ParseCalcLine.h"

#import <sstream>
#import <iomanip>
#import <cmath>

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self) {
    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		mCalcState = CreateCalcState();
		mFormatIntegersAsHex = NO;
		NSMenuItem*	optionsItem = [[NSApp mainMenu] itemWithTag: 500];
		if (optionsItem != nil)
		{
			NSMenu*	optionsMenu = [optionsItem submenu];
			if (optionsMenu != nil)
			{
				mCurFormatItem = [optionsMenu itemWithTag: 100];
			}
		}
		
		id attKeys[1] = {
			NSForegroundColorAttributeName
		};
		id attValues[1] = {
			[NSColor blackColor]
		};
		mNormalColorAtt = [[NSDictionary
			dictionaryWithObjects: attValues
			forKeys: attKeys
			count: 1] retain];
		
		attValues[0] = [NSColor redColor];
		mErrorColorAtt = [[NSDictionary
			dictionaryWithObjects: attValues
			forKeys: attKeys
			count: 1] retain];
		
		attValues[0] = [NSColor colorWithDeviceRed: 0.0
			green: 0.6
			blue: 0.0
			alpha: 1.0];
		mSuccessColorAtt = [[NSDictionary
			dictionaryWithObjects: attValues
			forKeys: attKeys
			count: 1] retain];
    }
    return self;
}

- (void)dealloc
{
	DisposeCalcState( mCalcState );
	
	[mNormalColorAtt release];
	[mErrorColorAtt release];
	[mSuccessColorAtt release];
	
	[super dealloc];
}

- (NSString *)windowNibName
{
    // Override returning the nib file name of the document
    // If you need to use a subclass of NSWindowController or if your document supports multiple NSWindowControllers, you should remove this method and override -makeWindowControllers instead.
    return @"MyDocument";
}

- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController has loaded the document's window.
	
	if (mString != nil) {
		[[textView textStorage] setAttributedString: mString];
		[self setString: nil];	// no further need for the mString member
	}
}

- (NSData *)dataRepresentationOfType:(NSString *)aType
{
    // Insert code here to write your document from the given data.  You can
	// also choose to override -fileWrapperRepresentationOfType: or
	// -writeToFile:ofType: instead.
    
    // For applications targeted for Tiger or later systems, you should use the
	// new Tiger API -dataOfType:error:.  In this case you can also choose to
	// override -writeToURL:ofType:error:, -fileWrapperOfType:error:, or
	// -writeToURL:ofType:forSaveOperation:originalContentsURL:error: instead.
	NSTextStorage*	theStorage = [textView textStorage];
	NSData*	theRTF = [theStorage
		RTFFromRange: NSMakeRange(0, [theStorage length])
		documentAttributes: nil];
	NSString*	theRTFString = [[NSString alloc]
		initWithBytes: [theRTF bytes]
		length: [theRTF length]
		encoding: NSUTF8StringEncoding];
	
	id	theKeys[3] = {
		@"text", @"variables", @"functions"
	};
	id	theValues[3] = {
		theRTFString,
		[NSDictionary dictionary],
		[NSDictionary dictionary]
	};
	// Currently I write an empty dictionary for variables and functions.
	// Eventually this will be a real dictionary.
	NSDictionary*	docDict = [NSDictionary dictionaryWithObjects: theValues
		forKeys: theKeys
		count: 3];
	[theRTFString release];
	NSString*	theError = nil;
	NSData*	data = [NSPropertyListSerialization
		dataFromPropertyList: docDict
		format: NSPropertyListXMLFormat_v1_0
		errorDescription: &theError ];
	if (theError != nil)
	{
		NSLog( theError );
		[theError release];
	}

    return data;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
	BOOL	didLoad;
	didLoad = NO;
    // Insert code here to read your document from the given data.  You can also
	// choose to override -loadFileWrapperRepresentation:ofType: or
	// -readFromFile:ofType: instead.
    
    // For applications targeted for Tiger or later systems, you should use the
	// new Tiger API readFromData:ofType:error:.  In this case you can also
	// choose to override -readFromURL:ofType:error: or
	// -readFromFileWrapper:ofType:error: instead.
	
	NSString*	errStr = nil;
	NSDictionary*	theDict = [NSPropertyListSerialization
		propertyListFromData: data
		mutabilityOption: NSPropertyListImmutable
		format: NULL
		errorDescription: &errStr ];
	if (errStr != nil)
	{
		NSLog( errStr );
		[errStr release];
	}
	if (theDict != nil)
	{
		NSString*	theRTFStr = [theDict objectForKey:@"text"];
		if (theRTFStr != NULL)
		{
			NSData*	theRTFdata = [theRTFStr
				dataUsingEncoding: NSUTF8StringEncoding];
			if (theRTFdata != nil)
			{
				NSAttributedString*	theAttStr = [[NSAttributedString alloc]
					initWithRTF: theRTFdata
					documentAttributes: NULL];
				if (theAttStr != NULL)
				{
					[self setString: theAttStr];
					didLoad = YES;
					[theAttStr release];
				}
			}
		}
	}
    
	return didLoad;
}

- (void) textDidChange: (NSNotification *) notification
{
	[self updateChangeCount: NSChangeDone];
}

- (void) insertString: (NSString*)string withAttributes: (NSDictionary*)dict
{
	NSAttributedString*	attStr = [[NSAttributedString alloc]
		initWithString: string
		attributes: dict ];
	[textView insertText: attStr];
	[attStr release];
}


- (NSString*) formatCalculatedResult: (double) value
{
	std::ostringstream	oss;
	if ( mFormatIntegersAsHex and
					(std::abs(value - round(value)) < FLT_EPSILON) )
	{
		oss << "0x" << std::hex << std::uppercase <<
						lround(value);
	}
	else
	{
		oss << std::setprecision(12) << value;
	}
	return [NSString stringWithUTF8String: oss.str().c_str() ];
}


- (BOOL)textView:(NSTextView *)aTextView doCommandBySelector:(SEL)aSelector
{
	BOOL	didHandle = NO;
	
	if ( (aTextView == textView) && (aSelector == @selector(insertNewline:)) )
	{
		NSRange	theRange = [textView selectedRange];
		if ((theRange.length == 0) && (theRange.location > 1))
		{
			NSTextStorage*	theStorage = [textView textStorage];
			NSString*	thePlainText = [theStorage string];

			unsigned int	lineStart = 0;
			unsigned int	lineEnd = theRange.location;
			NSCharacterSet*	linefeedSet = [NSCharacterSet
				characterSetWithCharactersInString: @"\n"];
			NSRange	prevBreak = [thePlainText
				rangeOfCharacterFromSet: linefeedSet
				options: NSBackwardsSearch
				range: NSMakeRange( 0, lineEnd )];
			if (prevBreak.length == 1)
			{
				lineStart = prevBreak.location + 1;
			}
			if (lineEnd > lineStart)
			{
				NSRange	lineRange = NSMakeRange( lineStart, lineEnd - lineStart );
				NSString*	theLine = [thePlainText
					substringWithRange: lineRange];
				const char*	lineCStr = [theLine UTF8String];

				// insert = and then line break
				[textView insertText: @" =\n"];
				
				// Pass the line to the calculator
				double	calculatedValue;
				long	stopOffset;
				ECalcResult	calcRes = ParseCalcLine( lineCStr, mCalcState,
					&calculatedValue, &stopOffset );
				
				if (calcRes == kCalcResult_Error)
				{
					[self insertString: @"Syntax Error"
						withAttributes: mErrorColorAtt ];
					
					[self insertString: @"\n"
						withAttributes: mNormalColorAtt ];
					
					[textView setSelectedRange:
						NSMakeRange( lineStart + stopOffset,
							lineEnd - lineStart - stopOffset) ];
				}
				else if (calcRes == kCalcResult_DefinedFunction)
				{
					[self insertString: @"Defined Function"
						withAttributes: mSuccessColorAtt ];
					
					[self insertString: @"\n"
						withAttributes: mNormalColorAtt ];
				}
				else
				{
					[self
						insertString:
							[self formatCalculatedResult: calculatedValue]
						withAttributes: mSuccessColorAtt ];
					
					[self insertString: @"\n"
						withAttributes: mNormalColorAtt ];
				}
				
				didHandle = YES;
			}
		}
	}
	
	return didHandle;
}

- (void) setIntegerFormat: (id) sender
{
	if ([sender isKindOfClass:[NSMenuItem class]])
	{
		NSMenuItem*	theItem = sender;
		[mCurFormatItem setState: NSOffState ];
		[theItem setState: NSOnState];
		mCurFormatItem = theItem;
		
		if ([theItem tag] == 101)
		{
			mFormatIntegersAsHex = YES;
		}
		else
		{
			mFormatIntegersAsHex = NO;
		}
	}
}

- (void) setString: (NSAttributedString*) newValue
{
	if (newValue != mString)
	{
		[mString release];
		mString = [newValue copy];
	}
}


@end

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

const OSType	kMyAppCreatorCode = 'PlCl';
const OSType	kMyNativeDocTypeCode = 'PlCl';

const int		kMenuTag_Options			= 500;
const int		kMenuItemTag_DecimalFormat	= 100;
const int		kMenuItemTag_HexFormat		= 101;

@implementation MyDocument

- (id)init
{
    self = [super init];
    if (self) {
    
        // Add your subclass-specific initialization here.
        // If an error occurs here, send a [self release] message and return nil.
		mCalcState = CreateCalcState();
		
		mFormatIntegersAsHex = NO;
		NSMenuItem*	optionsItem = [[NSApp mainMenu]
			itemWithTag: kMenuTag_Options ];
		if (optionsItem != nil)
		{
			NSMenu*	optionsMenu = [optionsItem submenu];
			if (optionsMenu != nil)
			{
				mDecFormatItem = [optionsMenu
					itemWithTag: kMenuItemTag_DecimalFormat];
				mHexFormatItem = [optionsMenu
					itemWithTag: kMenuItemTag_HexFormat];
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
    return @"MyDocument";
}


- (void)windowControllerDidLoadNib:(NSWindowController *) aController
{
    [super windowControllerDidLoadNib:aController];
    // Add any code here that needs to be executed once the windowController
	// has loaded the document's window.
	
	if (mString == nil)	// new doc
	{
		[textView setFont: [NSFont userFontOfSize: 14] ];
	}
	else	// opened existing doc
	{
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
	NSDictionary*	varDict = (NSDictionary*)CopyCalcVariables( mCalcState );
	NSDictionary*	funcDict = (NSDictionary*)CopyCalcFunctions( mCalcState );
	
	id	theKeys[3] = {
		@"text", @"variables", @"functions"
	};
	id	theValues[3] = {
		theRTFString,
		varDict,
		funcDict
	};
	NSDictionary*	docDict = [NSDictionary dictionaryWithObjects: theValues
		forKeys: theKeys
		count: 3];
	[theRTFString release];
	[varDict release];
	[funcDict release];
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

// Load data in our native document format
- (BOOL) loadNativeData:(NSData *)data
{
	BOOL	didLoad = NO;
	
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
		NSDictionary*	varDict = [theDict objectForKey: @"variables"];
		if (varDict != NULL)
		{
			SetCalcVariables( (CFDictionaryRef)varDict, mCalcState );
		}
		NSDictionary*	funcDict = [theDict objectForKey: @"functions"];
		if (funcDict != NULL)
		{
			SetCalcFunctions( (CFDictionaryRef)funcDict, mCalcState );
		}
	}
    
	return didLoad;
}

- (BOOL) loadPlainTextData: (NSData *)data
{
	BOOL	didLoad = NO;
	NSString*	textStr = [[NSString alloc] initWithData: data
		encoding: NSUTF8StringEncoding ];
	NSAttributedString*	theAttStr = [[NSAttributedString alloc]
		initWithString: textStr ];
	[textStr release];
	[self setString: theAttStr];
	didLoad = YES;
	[theAttStr release];
	
	return didLoad;
}

- (BOOL) loadRTFData: (NSData *)data
{
	BOOL	didLoad = NO;
	NSAttributedString*	theAttStr = [[NSAttributedString alloc]
		initWithRTF: data
		documentAttributes: NULL];
	if (theAttStr != NULL)
	{
		[self setString: theAttStr];
		didLoad = YES;
		[theAttStr release];
	}
	return didLoad;
}

- (BOOL)loadDataRepresentation:(NSData *)data ofType:(NSString *)aType
{
	BOOL	didLoad = NO;
	
	if ([aType compare: NSStringPboardType] == NSOrderedSame)
	{
		didLoad = [self loadPlainTextData: data];
	}
 	else if ([aType compare: NSRTFPboardType] == NSOrderedSame)
	{
		didLoad = [self loadRTFData: data];
	}
	else
	{
		didLoad = [self loadNativeData: data];
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
		(value > 0.0) and
		(value < 4.6e+15) and
		(fabs(value - round(value)) < FLT_EPSILON) )
	{
		oss << "0x" << std::hex << std::uppercase <<
						llround(value);
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

- (void)setIntegerFormatChecks
{
	if (mFormatIntegersAsHex)
	{
		[mDecFormatItem setState: NSOffState];
		[mHexFormatItem setState: NSOnState];
	}
	else
	{
		[mDecFormatItem setState: NSOnState];
		[mHexFormatItem setState: NSOffState];
	}
}

- (void)windowDidBecomeMain:(NSNotification *)aNotification
{
	[self setIntegerFormatChecks];
}

- (void) setIntegerFormat: (id) sender
{
	if ([sender isKindOfClass:[NSMenuItem class]])
	{
		NSMenuItem*	theItem = sender;

		if ([theItem tag] == kMenuItemTag_HexFormat)
		{
			mFormatIntegersAsHex = YES;
		}
		else
		{
			mFormatIntegersAsHex = NO;
		}
		
		[self setIntegerFormatChecks];
	}
}

- (void) showDefinedVariables: (id) sender
{
	NSDictionary*	varDict = (NSDictionary*)CopyCalcVariables( mCalcState );
	NSArray*	theKeys = [[varDict allKeys]
		sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
	NSEnumerator *enumerator = [theKeys objectEnumerator];
	
	NSMutableString* theStr = [[NSMutableString alloc] initWithCapacity:100];
	[theStr appendString: @"Defined Variables:" ];
	
	id key;
	while ((key = [enumerator nextObject]) != nil)
	{
		id theValue = [varDict valueForKey: key];
		if (theValue != nil)
		{
			NSString*	valueStr = [self formatCalculatedResult:
				[theValue doubleValue] ];
			[theStr appendFormat: @"\n%@ = %@", key, valueStr ];
		}
	}
	[varDict release];
	
	[self insertString: theStr
		withAttributes: mSuccessColorAtt ];
	[theStr release];
	
	[self insertString: @"\n"
		withAttributes: mNormalColorAtt ];
}

- (void) showDefinedFunctions: (id) sender
{
	NSDictionary*	funcDict = (NSDictionary*)CopyCalcFunctions( mCalcState );
	NSArray*	theKeys = [[funcDict allKeys]
		sortedArrayUsingSelector: @selector(caseInsensitiveCompare:)];
	NSEnumerator *enumerator = [theKeys objectEnumerator];
	
	NSMutableString* theStr = [[NSMutableString alloc] initWithCapacity:100];
	[theStr appendString: @"Defined Functions:" ];

	id key;
	while ((key = [enumerator nextObject]) != nil)
	{
		[theStr appendFormat: @"\n%@( ", key ];
		NSArray* theValue = (NSArray*)[funcDict valueForKey: key];
		for (int i = 1; i < [theValue count]; ++i)
		{
			if (i > 1)
			{
				[theStr appendFormat: @", %@", [theValue objectAtIndex: i] ];
			}
			else
			{
				[theStr appendFormat: @"%@", [theValue objectAtIndex: i] ];
			}
		}
		[theStr appendFormat: @" ) = %@", [theValue objectAtIndex: 0] ];
	}
	
	[funcDict release];

	[self insertString: theStr
		withAttributes: mSuccessColorAtt ];
	[theStr release];
	
	[self insertString: @"\n"
		withAttributes: mNormalColorAtt ];
}


- (void) setString: (NSAttributedString*) newValue
{
	if (newValue != mString)
	{
		[mString release];
		mString = [newValue copy];
	}
}

- (void)printOperationDidRun:(NSPrintOperation *)printOperation
                success:(BOOL)success
                contextInfo:(void *)info
{
	
}

- (void)printShowingPrintPanel:(BOOL)flag
{
	NSPrintInfo*	thePrintInfo = [self printInfo];
	
	[thePrintInfo setVerticallyCentered: NO ];
	
	NSPrintOperation *op = [NSPrintOperation
                printOperationWithView: textView
                printInfo: thePrintInfo ];

	[op runOperationModalForWindow: docWindow
                delegate: self
                didRunSelector:
                    @selector(printOperationDidRun:success:contextInfo:)
                contextInfo: self];
}

- (NSDictionary *)fileAttributesToWriteToURL:(NSURL *)absoluteURL
    ofType:(NSString *)typeName
    forSaveOperation:(NSSaveOperationType)saveOperation
    originalContentsURL:(NSURL *)absoluteOriginalContentsURL
    error:(NSError **)outError
{
    NSMutableDictionary *fileAttributes =
            [[super fileAttributesToWriteToURL:absoluteURL
             ofType:typeName forSaveOperation:saveOperation
             originalContentsURL:absoluteOriginalContentsURL
             error:outError] mutableCopy];
    [fileAttributes
		setObject: [NSNumber numberWithUnsignedInt: kMyAppCreatorCode]
        forKey: NSFileHFSCreatorCode];
    [fileAttributes
		setObject: [NSNumber numberWithUnsignedInt: kMyNativeDocTypeCode]
        forKey: NSFileHFSTypeCode];
    return [fileAttributes autorelease];
}

@end

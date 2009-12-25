//
//  MyDocument.m
//  PlainCalc2
//
//  Created by James Walker on 3/11/06.
//  Copyright James W. Walker 2006 . All rights reserved.
//

#import "MyDocument.h"
#import "GetDefaultFont.h"
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
	
	[mInitialTypingFont release];
	[mNormalColorAtt release];
	[mErrorColorAtt release];
	[mSuccessColorAtt release];
	
	[super dealloc];
}


- (void) setString: (NSAttributedString*) newValue
{
	if (newValue != mString)
	{
		[mString release];
		mString = [newValue copy];
	}
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
		
		NSString* fontName = [theDict objectForKey: @"fontName"];
		NSNumber* fontSize = [theDict objectForKey: @"fontSize"];
		if ( (fontName != nil) and (fontSize != nil) )
		{
			NSFontManager*	fontMgr = [NSFontManager sharedFontManager];
			NSFont* typingFont = [fontMgr
				fontWithFamily: fontName
				traits: 0
				weight: 5
				size: [fontSize floatValue] ];
			if (typingFont != nil)
			{
				mInitialTypingFont = [typingFont retain];
			}
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


- (void)printOperationDidRun:(NSPrintOperation *)printOperation
                success:(BOOL)success
                contextInfo:(void *)info
{
	
}

#pragma mark NSDocument overloads

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
		NSFont* myFont = GetDefaultFont();

		[textView setFont: myFont ];
	}
	else	// opened existing doc
	{
		[[textView textStorage] setAttributedString: mString];
		[self setString: nil];	// no further need for the mString member
		
		NSMutableDictionary* typingAtts = [NSMutableDictionary
			dictionaryWithDictionary: [textView typingAttributes] ];
		[typingAtts setValue: mInitialTypingFont
					forKey: NSFontAttributeName];
		[textView setTypingAttributes: typingAtts];
	}
}

- (NSData *)dataOfType:(NSString *)typeName
			error:(NSError **)outError
{
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
	
	NSDictionary*	typingAtts = [textView typingAttributes];
	NSFont* typingFont = [typingAtts valueForKey: NSFontAttributeName];
	NSString* typingFontName = [typingFont familyName];
	NSNumber* typingFontSize = [NSNumber numberWithFloat: [typingFont pointSize] ];
	
	id	theKeys[] = {
		@"text", @"variables", @"functions", @"fontName", @"fontSize"
	};
	id	theValues[] = {
		theRTFString,
		varDict,
		funcDict,
		typingFontName,
		typingFontSize
	};
	NSDictionary*	docDict = [NSDictionary dictionaryWithObjects: theValues
		forKeys: theKeys
		count: 5];
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
		
		if (outError)
		{
			*outError = [NSError errorWithDomain: NSCocoaErrorDomain
								code: NSFileWriteUnknownError
								userInfo: nil];
		}
	}

    return data;
}

- (BOOL)readFromData:(NSData *)data
		ofType:(NSString *)typeName
		error:(NSError **)outError
{
	BOOL	didLoad = NO;
	
	if ([typeName isEqualToString: NSStringPboardType])
	{
		didLoad = [self loadPlainTextData: data];
	}
 	else if ([typeName isEqualToString: NSRTFPboardType])
	{
		didLoad = [self loadRTFData: data];
	}
	else if ([typeName isEqualToString: @"PlainCalc worksheet"])
	{
		didLoad = [self loadNativeData: data];
	}
	else
	{
		if (outError)
		{
			*outError = [NSError errorWithDomain: NSCocoaErrorDomain
								code: NSFileReadCorruptFileError
								userInfo: nil];
		}
	}
   
	return didLoad;
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

- (void)canCloseDocumentWithDelegate:(id)delegate
		shouldCloseSelector:(SEL)shouldCloseSelector
		contextInfo:(void *)contextInfo;
{
	if ( (not [self isDocumentEdited]) or
		([self fileURL] == nil) )
	{
		NSMethodSignature* theSig = [delegate methodSignatureForSelector:
			shouldCloseSelector];
		NSInvocation* invoc = [NSInvocation invocationWithMethodSignature: theSig];
		[invoc setSelector: shouldCloseSelector];
		[invoc setTarget: delegate];
		[invoc setArgument: &self atIndex: 2];
		BOOL shouldClose = YES;
		[invoc setArgument: &shouldClose atIndex: 3];
		[invoc setArgument: &contextInfo atIndex: 4];
		[invoc invoke];
	}
	else
	{
		[super canCloseDocumentWithDelegate: delegate
				shouldCloseSelector: shouldCloseSelector
				contextInfo: contextInfo];
	}
}

#pragma mark NSText delegate

- (void) textDidChange: (NSNotification *) notification
{
	[self updateChangeCount: NSChangeDone];
}

#pragma mark NSTextView delegate

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

- (NSDictionary *)textView:(NSTextView *)textView
				shouldChangeTypingAttributes:(NSDictionary *) oldTypingAttributes
				toAttributes:(NSDictionary *)newTypingAttributes
{
	NSDictionary * attsToUse = newTypingAttributes;
	
	NSColor* colorAtt = [attsToUse valueForKey: NSForegroundColorAttributeName];
	
	if ( (colorAtt != nil) and (not [colorAtt isEqual: [NSColor blackColor]]) )
	{
		attsToUse = [NSMutableDictionary dictionaryWithDictionary: attsToUse];
		[attsToUse setValue:[NSColor blackColor]
					forKey: NSForegroundColorAttributeName];
	}
	
	return attsToUse;
}

#pragma mark NSWindow delegate

- (void)windowDidBecomeMain:(NSNotification *)aNotification
{
	[self setIntegerFormatChecks];
}


#pragma mark Actions

- (IBAction) setIntegerFormat: (id) sender
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


- (IBAction) showDefinedVariables: (id) sender
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

- (IBAction) showDefinedFunctions: (id) sender
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

- (IBAction) pasteCleaned: (id) sender
{
	NSString* dataType = [[NSPasteboard generalPasteboard]
		availableTypeFromArray:
			[NSArray arrayWithObject: NSStringPboardType] ];
	
	if (dataType != nil)
	{
		NSString* theData = [[NSPasteboard generalPasteboard]
			stringForType: NSStringPboardType ];
		
		NSCharacterSet* badChars = [NSCharacterSet
			characterSetWithCharactersInString: @",$"];
		
		const unsigned int kNumChars = [theData length];
		
		NSMutableString* cleanData = [NSMutableString stringWithCapacity:
			kNumChars ];
		
		for (unsigned int i = 0; i < kNumChars; ++i)
		{
			unichar aChar = [theData characterAtIndex: i];
			
			if (not [badChars characterIsMember: aChar])
			{
				[cleanData appendString:
					[NSString stringWithCharacters: &aChar length: 1] ];
			}
		}
		
		[textView insertText: cleanData];
	}
}

@end

//
//  MyDocument.m
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

#import "MyDocument.h"
#import "ParseCalcLine.h"
#import "AppController.h"
#import "NSString_JW.h"


#import <sstream>
#import <iomanip>
#import <cmath>

const OSType	kMyAppCreatorCode = 'PlCl';
const OSType	kMyNativeDocTypeCode = 'PlCl';

const int		kMenuTag_Options			= 500;
const int		kMenuItemTag_DecimalFormat	= 100;
const int		kMenuItemTag_HexFormat		= 101;

@interface MyDocument ()

- (void) startCalcTask;

@end

@implementation MyDocument

- (NSString*) standardizeString: (NSString*) localStr
{
	NSString* stdString = localStr;
	
	if ([AppController isCommaDecimal])
	{
		NSString* workStr = [localStr stringByReplacingChar: ',' byChar: '.'];
		stdString = [workStr stringByReplacingChar: ';' byChar: ','];
	}
	
	return stdString;
}

- (NSString*) localizeString: (NSString*) stdString
{
	NSString* locString = stdString;
	
	if ([AppController isCommaDecimal])
	{
		NSString* workStr = [stdString stringByReplacingChar: ',' byChar: ';'];
		locString = [workStr stringByReplacingChar: '.' byChar: ','];
	}
	
	return locString;
}

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
		
		mResultBuffer = [[NSMutableData alloc] init];
		mLineToCalculate = [[NSMutableString alloc] init];
    }
    return self;
}

- (void)dealloc
{
	NSLog(@"dealloc 1");
	[[NSNotificationCenter defaultCenter] removeObserver: self];
	
	DisposeCalcState( mCalcState );
	NSLog(@"dealloc 2");
	[mCalcTask terminate];
	NSLog(@"dealloc 3");
	
	[mInitialTypingFont release];
	[mLoadedWindowFrame release];
	[mCalcTask release];
	[mResultBuffer release];
	[mLineToCalculate release];
	NSLog(@"dealloc 4");
	
	[super dealloc];
	NSLog(@"dealloc 5");
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
		NSLog( @"Error: %@", errStr );
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
		
		mLoadedWindowFrame = [[theDict objectForKey: @"windowFrame"] retain];
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
	return [self localizeString:
		[NSString stringWithUTF8String: oss.str().c_str() ] ];
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

- (NSString*) pathOfNewDocState
{
	return [@"~/Library/Preferences/com.jwwalker.PlainCalc2.plaincalc"
		stringByExpandingTildeInPath];
}

- (NSData*) dataOfNewDoc
{
	NSData* theData = [NSData dataWithContentsOfFile: [self pathOfNewDocState]];
	return theData;
}

- (void) setLoadedDocData
{
	[[textView textStorage] setAttributedString: mString];
	[self setString: nil];	// no further need for the mString member
	
	NSMutableDictionary* typingAtts = [NSMutableDictionary
		dictionaryWithDictionary: [textView typingAttributes] ];
	[typingAtts setValue: mInitialTypingFont
				forKey: NSFontAttributeName];
	[textView setTypingAttributes: typingAtts];
}

- (void) handleTaskResult: (NSDictionary*) dict
{
	[textView setEditable: YES];
	[NSObject cancelPreviousPerformRequestsWithTarget: self];

	NSRange	theRange = [textView selectedRange];
	NSUInteger	lineEnd = theRange.location;
	NSUInteger	lineStart = lineEnd - [mLineToCalculate length];
	
	ECalcResult resultKind = (ECalcResult) [[dict valueForKey: @"ResultKind"]
		intValue];
	
	if (resultKind == kCalcResult_Error)
	{
		long	stopOffset = [[dict valueForKey: @"Stop"] intValue];
		[self insertString: NSLocalizedString( @"SynErr", nil )
			withAttributes: [AppController errorAtts] ];
		
		[self insertString: @"\n"
			withAttributes: [AppController normalAtts] ];
		
		[textView setSelectedRange:
			NSMakeRange( lineStart + stopOffset,
				lineEnd - lineStart - stopOffset) ];
	}
	else if (resultKind == kCalcResult_DefinedFunction)
	{
		[self insertString: NSLocalizedString( @"DefFun", nil )
			withAttributes: [AppController successAtts] ];
		
		[self insertString: @"\n"
			withAttributes: [AppController normalAtts] ];

		// Shadow the function in the app's calculator
		double	calculatedValue;
		long	stopOffset;
		(void) ParseCalcLine( [mLineToCalculate UTF8String],
			mCalcState, &calculatedValue, &stopOffset );
	}
	else // kCalcResult_Calculated
	{
		double result = [[dict valueForKey: @"Result"] doubleValue];

		[self	insertString: [self formatCalculatedResult: result]
				withAttributes: [AppController successAtts] ];
		
		[self insertString: @"\n"
			withAttributes: [AppController normalAtts] ];

		// Shadow the value in the app's calculator
		SetCalcVariable( "last", result, mCalcState );
		NSString* symbol = [dict valueForKey: @"Symbol"];
		if ([symbol length] > 0)
		{
			SetCalcVariable( [symbol UTF8String], result, mCalcState );
		}
	}
}

- (void) receiveDataFromTask: (NSNotification *)aNotification
{
	NSDictionary* userInfo = [aNotification userInfo];
	NSData *data = [userInfo objectForKey: NSFileHandleNotificationDataItem];
	if ([data length])
	{
		[mResultBuffer appendData: data];
		NSString* replyStr = [[[NSString alloc]
			initWithData: mResultBuffer
			encoding: NSUTF8StringEncoding] autorelease];
		NSRange foundRange = [replyStr rangeOfString: @"</plist>"
										options: NSBackwardsSearch];
		if (foundRange.length > 0)
		{
			NSString* errDesc = nil;
			NSDictionary* replyDict = (NSDictionary*)
				[NSPropertyListSerialization
					propertyListFromData: mResultBuffer
					mutabilityOption: NSPropertyListImmutable
					format: NULL
					errorDescription: &errDesc];
			[errDesc release];
			[mResultBuffer setLength: 0];
			if (replyDict != nil)
			{
				[self handleTaskResult: replyDict];
			}
		}
		
		// we need to schedule the file handle go read more data in the background again.
		[[aNotification object] readInBackgroundAndNotify];  
	}
}

- (void) taskEnded: (NSNotification *)aNote
{
	NSTask* theTask = [aNote object];
	if (theTask == mCalcTask)
	{
		int status = [theTask terminationStatus];
		[NSObject cancelPreviousPerformRequestsWithTarget: self];
		if ( (status != 0) and (status != SIGTERM) )
		{
			[textView setEditable: YES];
			
			[self insertString: NSLocalizedString( @"Crash", nil )
				withAttributes: [AppController errorAtts] ];
			
			[self insertString: @"\n"
				withAttributes: [AppController normalAtts] ];
		}
		
		[mCalcTask release];
		mCalcTask = nil;
		[self startCalcTask];
	}
}

- (NSString*) xmlStringFromDictionary: (NSDictionary*) dict
{
	NSString* errDesc = nil;
	NSData* xmlData = [NSPropertyListSerialization
		dataFromPropertyList: dict
		format: NSPropertyListXMLFormat_v1_0
		errorDescription: &errDesc];
	[errDesc release];
	NSString* theXMLStr = [[[NSString alloc]
		initWithData: xmlData
		encoding: NSUTF8StringEncoding] autorelease];
	return theXMLStr;
}

- (void) startCalcTask
{
	mCalcTask = [[NSTask alloc] init];
	NSString* toolPath = [[[[NSBundle mainBundle] executablePath]
		stringByDeletingLastPathComponent]
		stringByAppendingPathComponent: @"CalcTool"];
	[mCalcTask setLaunchPath: toolPath];

	NSDictionary*	varDict = (NSDictionary*)CopyCalcVariables( mCalcState );
	[varDict autorelease];
	NSString* varXMLStr = [self xmlStringFromDictionary: varDict];
	
	NSDictionary*	funcDict = (NSDictionary*)CopyCalcFunctions( mCalcState );
	[funcDict autorelease];
	NSString* funXMLStr = [self xmlStringFromDictionary: funcDict];

	NSArray* argArray = [NSArray arrayWithObjects:
		varXMLStr, funXMLStr, nil ];
	[mCalcTask setArguments: argArray];
	
	[mCalcTask setStandardInput: [NSPipe pipe] ];
	[mCalcTask setStandardOutput: [NSPipe pipe] ];
	
	// Wait for output in the background, and notify when data is ready.
	NSFileHandle* outFileHandle = [[mCalcTask standardOutput] fileHandleForReading];
    [[NSNotificationCenter defaultCenter]
		addObserver: self 
        selector: @selector(receiveDataFromTask:) 
        name: NSFileHandleReadCompletionNotification 
        object: outFileHandle ];
	[outFileHandle readInBackgroundAndNotify];
	
	[[NSNotificationCenter defaultCenter]
		addObserver: self
		selector: @selector(taskEnded:)
		name: NSTaskDidTerminateNotification
		object: nil ];
	
	@try
	{
		[mCalcTask launch];
	}
	@catch (NSException *exception)
	{
		NSLog(@"Caught %@: %@", [exception name], [exception  reason]);
	}
}

- (void) sendCommandToTask: (NSString*) calcStr
{
	NSString* commandStr = [calcStr stringByAppendingString: @"\n"];
	const char* theCString = [commandStr UTF8String];
	size_t theLen = strlen( theCString );
	NSData* theData = [NSData dataWithBytes: theCString length: theLen ];
	
	NSPipe* thePipe = [mCalcTask standardInput];
	NSFileHandle* theFileHandle = [thePipe fileHandleForWriting];
	[theFileHandle writeData: theData ];
}

- (void) calcTimedOut
{
	[[NSNotificationCenter defaultCenter] removeObserver: self];
	
	[mCalcTask terminate];
	[mCalcTask release];
	mCalcTask = nil;
	
	[self startCalcTask];
	
	[mResultBuffer setLength: 0];
	
	[textView setEditable: YES];
	
	[self insertString: NSLocalizedString( @"Timeout", nil )
		withAttributes: [AppController errorAtts] ];
	
	[self insertString: @"\n"
		withAttributes: [AppController normalAtts] ];
}

#pragma mark Sheet end callbacks

- (void) infoAlertEnd:(NSAlert *)alert
		retCode:(int)returnCode
		ctx:(void *)contextInfo
{
	
}

- (void) saveAsNewConfirmEnd:(NSAlert *)alert
		retCode:(int)returnCode
		ctx:(void *)contextInfo
{
	if (returnCode == NSAlertFirstButtonReturn)
	{
		[[alert window] orderOut: self];
		
		NSData* theData = [self dataOfType: @"PlainCalc worksheet" error: nil];
		
		if (theData)
		{
			NSError* theErr = nil;
			
			if (not [theData writeToFile: [self pathOfNewDocState]
							options: NSAtomicWrite
							error: &theErr ])
			{
				NSAlert* errAlert = [NSAlert alertWithError: theErr];
				[errAlert beginSheetModalForWindow: docWindow
							modalDelegate: self
							didEndSelector: @selector(infoAlertEnd:retCode:ctx:)
							contextInfo: nil];
			}
		}
	}
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
		NSData* theData = [self dataOfNewDoc];
		
		if ( (theData != nil) and [self loadNativeData: theData] )
		{
			[self setLoadedDocData];
		}
		else
		{
			[textView setFont: [NSFont userFontOfSize: 14] ];
		}
	}
	else	// opened existing doc
	{
		[self setLoadedDocData];
	}
	
	if (mLoadedWindowFrame)
	{
		[docWindow setFrameFromString: mLoadedWindowFrame];
	}
	
	[self startCalcTask];
}

- (NSData *)dataOfType:(NSString *)typeName
			error:(NSError **)outError
{
	NSTextStorage*	theStorage = [textView textStorage];
	NSData*	theRTF = [theStorage
		RTFFromRange: NSMakeRange(0, [theStorage length])
		documentAttributes: @{}];
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
		@"text", @"variables", @"functions", @"fontName", @"fontSize",
		@"windowFrame"
	};
	id	theValues[] = {
		theRTFString,
		varDict,
		funcDict,
		typingFontName,
		typingFontSize,
		[docWindow stringWithSavedFrame]
	};
	NSDictionary*	docDict = [NSDictionary
		dictionaryWithObjects: theValues
		forKeys: theKeys
		count: 6];
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
		NSLog( @"Error: %@", theError );
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

- (void)printDocumentWithSettings:(NSDictionary<NSPrintInfoAttributeKey, id> *)printSettings
	showPrintPanel:(BOOL)showPrintPanel
	delegate:(id)delegate
	didPrintSelector:(SEL)didPrintSelector
	contextInfo:(void *)contextInfo
{
	NSPrintInfo*	thePrintInfo = [self printInfo];
	
	[thePrintInfo setVerticallyCentered: NO ];
	
	NSPrintOperation *op = [NSPrintOperation
                printOperationWithView: textView
                printInfo: thePrintInfo ];

	[op runOperationModalForWindow: docWindow
                delegate: delegate
                didRunSelector:  didPrintSelector
                contextInfo: contextInfo];
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
		NSLog(@"canCloseDocumentWithDelegate 1");
		[super canCloseDocumentWithDelegate: delegate
				shouldCloseSelector: shouldCloseSelector
				contextInfo: contextInfo];
		NSLog(@"canCloseDocumentWithDelegate 2");
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
		if ((theRange.length == 0) && (theRange.location > 0))
		{
			NSTextStorage*	theStorage = [textView textStorage];
			NSString*	thePlainText = [theStorage string];

			NSUInteger	lineStart = 0;
			NSUInteger	lineEnd = theRange.location;
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
				theLine = [self standardizeString: theLine];
				[mLineToCalculate setString: theLine];

				// insert = and then line break
				[textView insertText: @" =\n"];
				[textView setEditable: NO];
				
				[self sendCommandToTask: theLine];
				
				[self performSelector: @selector(calcTimedOut)
						withObject: nil
						afterDelay: [AppController calcTimeout]];
				
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
	[theStr appendString: NSLocalizedString( @"DefVars", nil ) ];
	
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
		withAttributes: [AppController successAtts] ];
	[theStr release];
	
	[self insertString: @"\n"
		withAttributes: [AppController normalAtts] ];
}

- (IBAction) showDefinedFunctions: (id) sender
{
	NSDictionary*	funcDict = (NSDictionary*)CopyCalcFunctions( mCalcState );
	NSArray*	theKeys = [[funcDict allKeys]
		sortedArrayUsingSelector: @selector(caseInsensitiveCompare:)];
	NSEnumerator *enumerator = [theKeys objectEnumerator];
	
	NSMutableString* theStr = [[NSMutableString alloc] initWithCapacity:100];
	[theStr appendString: NSLocalizedString( @"DefFuns", nil ) ];
	
	NSString* fmtWithSep = [AppController isCommaDecimal]?
		@"; %@" : @", %@";

	id key;
	while ((key = [enumerator nextObject]) != nil)
	{
		[theStr appendFormat: @"\n%@( ", key ];
		NSArray* theValue = (NSArray*)[funcDict valueForKey: key];
		for (int i = 1; i < [theValue count]; ++i)
		{
			if (i > 1)
			{
				[theStr appendFormat: fmtWithSep, [theValue objectAtIndex: i] ];
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
		withAttributes: [AppController successAtts] ];
	[theStr release];
	
	[self insertString: @"\n"
		withAttributes: [AppController normalAtts] ];
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
		
		const NSUInteger kNumChars = [theData length];
		
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

- (IBAction) saveAsNewDocumentContent: (id) sender
{
	NSAlert* theAlert = [[[NSAlert alloc] init] autorelease];
	[theAlert setMessageText: NSLocalizedString( @"ConfirmSaveNew", nil )];
	[theAlert setInformativeText: NSLocalizedString( @"ConfirmSaveNew_detail", nil )];
	[theAlert addButtonWithTitle: NSLocalizedString( @"ConfirmSaveNew_ok", nil )];
	[theAlert addButtonWithTitle: NSLocalizedString( @"ConfirmSaveNew_cancel", nil )];
	
	[theAlert beginSheetModalForWindow: docWindow
		modalDelegate: self
		didEndSelector: @selector(saveAsNewConfirmEnd:retCode:ctx:)
		contextInfo: nil];
}

- (IBAction) forgetFunction: (id) sender
{
	mForgettingFunction = YES;
	[oForgetSymbolPopup removeAllItems];
	NSDictionary*	funcDict = (NSDictionary*)CopyCalcFunctions( mCalcState );
	[funcDict autorelease];
	NSArray*	theKeys = [[funcDict allKeys]
		sortedArrayUsingSelector: @selector(caseInsensitiveCompare:)];
	[oForgetSymbolPopup addItemsWithTitles: theKeys];
	
	[NSApp beginSheet: oForgetSymbolSheet
			modalForWindow: docWindow
			modalDelegate: nil
			didEndSelector: nil
			contextInfo: NULL ];
}

- (IBAction) forgetVariable: (id) sender
{
	mForgettingFunction = NO;
	[oForgetSymbolPopup removeAllItems];
	NSDictionary*	varDict = (NSDictionary*)CopyCalcVariables( mCalcState );
	[varDict autorelease];
	NSArray*	theKeys = [[varDict allKeys]
		sortedArrayUsingSelector: @selector(caseInsensitiveCompare:)];
	[oForgetSymbolPopup addItemsWithTitles: theKeys];
	
	[NSApp beginSheet: oForgetSymbolSheet
			modalForWindow: docWindow
			modalDelegate: nil
			didEndSelector: nil
			contextInfo: NULL ];
}

- (IBAction) forgetSheetOK: (id) sender
{
	NSDictionary*	funcDict = (NSDictionary*)CopyCalcFunctions( mCalcState );
	[funcDict autorelease];
	NSDictionary*	varDict = (NSDictionary*)CopyCalcVariables( mCalcState );
	[varDict autorelease];
	NSString* symbolName = [oForgetSymbolPopup titleOfSelectedItem];
	
	if (mForgettingFunction)
	{
		NSMutableDictionary* funcsMutable = [[funcDict mutableCopy] autorelease];
		[funcsMutable removeObjectForKey: symbolName];
		funcDict = funcsMutable;
	}
	else
	{
		NSMutableDictionary* varsMutable = [[varDict mutableCopy] autorelease];
		[varsMutable removeObjectForKey: symbolName];
		varDict = varsMutable;
	}
	
	CalcState newCalcState = CreateCalcState();
	SetCalcVariables( (CFDictionaryRef)varDict, newCalcState );
	SetCalcFunctions( (CFDictionaryRef)funcDict, newCalcState );
	DisposeCalcState( mCalcState );
	mCalcState = newCalcState;

	[NSApp endSheet: oForgetSymbolSheet];
	[oForgetSymbolSheet orderOut: self];
}

- (IBAction) forgetSheetCancel: (id) sender
{
	[NSApp endSheet: oForgetSymbolSheet];
	[oForgetSymbolSheet orderOut: self];
}


@end

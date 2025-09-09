//  Document.m
//  
//
//  Created by James Walker on 8/16/25.
//  
//
/*
	Copyright (c) 2025 James W. Walker

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

#import "Document.h"

#import "AppDelegate.h"
#import "Calculate.hpp"
#import "LoadStateFromDictionary.h"
#import "PerformBlockOnWorkThread.h"
#import "SaveStateToDictionary.h"
#import "SCalcState.hpp"
#import "UTF8toUTF32.hpp"
#import "UTF32toUTF16.hpp"

#import <sstream>
#import <iomanip>
#import <assert.h>

@interface Document ()
{
	IBOutlet NSTextView*	_textView;
	IBOutlet NSWindow*		_docWindow;
	IBOutlet NSView*		_calculatingOverlay;
	IBOutlet NSWindow*		_deleteSymbolSheet;
	IBOutlet NSPopUpButton*	_deleteSymbolPopup;
	IBOutlet NSButton*		_deleteSymbolOKButton;
}

@end

@implementation Document
{
	NSAttributedString*		_initialText;
	NSFont*					_initialTypingFont;
	NSString*				_initialWindowFrame;
	SCalcState				_calcState;
	BOOL					_formatIntegersAsHex;
	NSRange					_lastCalculatedLineRange;
}

- (instancetype) init
{
    self = [super init];
    if (self)
    {
		_initialText = nil;
		_initialTypingFont = [NSFont fontWithName: @"Helvetica" size: 14.0 ];
    }
    return self;
}

#if DEBUG
- (void) dealloc
{
	NSLog(@"dealloc Document");
}
#endif

+ (BOOL) autosavesInPlace
{
	return NO;
}

- (NSString*) formatCalculatedResult: (double) value
{
	std::ostringstream	oss;
	if ( _formatIntegersAsHex and
		(value > 0.0) and
		(value < 4.6e+15) and
		(fabs(value - round(value)) < FLT_EPSILON) )
	{
		oss << "0x" << std::hex << std::uppercase <<
						llround(value);
	}
	else if ( _formatIntegersAsHex and
		(value < 0.0) and
		(value >= -2147483648.0) and
		(fabs(value - round(value)) < FLT_EPSILON) )
	{
		oss << "0x" << std::hex << std::uppercase <<
						(4294967296 + lround(value));
	}
	else
	{
		oss << std::setprecision(12) << value;
	}
	std::string rawResult( oss.str() );
	if (rawResult == "inf")
	{
		rawResult = "infinity";
	}
	else if (rawResult == "-inf")
	{
		rawResult = "-infinity";
	}
	else if (rawResult == "nan")
	{
		rawResult = "undefined";
	}
	return [self localizeString: @( rawResult.c_str() ) ];
}

- (void) insertString: (NSString*)string withAttributes: (NSDictionary*)dict
{
	NSAttributedString*	attStr = [[NSAttributedString alloc]
		initWithString: string
		attributes: dict ];
	[_textView insertText: attStr
		replacementRange: _textView.selectedRange];
}

- (NSURL*) newDocStateURL
{
	NSURL* appSuppURL = [NSFileManager.defaultManager
		URLForDirectory: NSApplicationSupportDirectory
		inDomain: NSUserDomainMask
		appropriateForURL: nil
		create: YES
		error: nil];
	NSURL* fileURL = [appSuppURL
		URLByAppendingPathComponent: @"PlainCalc3-new-doc-template.plaincalc"
		isDirectory: NO];
	return fileURL;
}

- (NSColor*) correctColor: (NSColor*) color
{
	NSColor* fixedColor = nil;
	if ( (color != nil) and (not [color isEqualTo: NSColor.textColor]) )
	{
		NSColor* origRGB = [color
			colorUsingColorSpace: NSColorSpace.genericRGBColorSpace];
		if (origRGB != nil)
		{
			CGFloat r, g, b, a;
			[origRGB getRed: &r green: &g blue: &b alpha: &a];
			if ( (g > r + 0.4) and (g > b + 0.4) ) // mostly green
			{
				fixedColor = [NSColor colorNamed: @"GoodResult"];
			}
			else if ( (r > g + 0.4) and (r > b + 0.4) ) // mostly red
			{
				fixedColor = [NSColor colorNamed: @"BadResult"];
			}
		}
	}
	
	return fixedColor;
}

/*
	For some reason, saving to RTF preserves the adaptable color for text,
	but not not my asset catalog colors for success and failure.  Hence this
	hack to fix the text and underline colors.
 */
- (void) correctTextColors: (NSMutableAttributedString*) attStr
{
	[attStr enumerateAttribute: NSForegroundColorAttributeName
				inRange: NSMakeRange( 0, attStr.length )
				options: 0
				usingBlock:
		^(id  _Nullable value, NSRange range, BOOL * _Nonnull stop)
		{
			NSColor* origColor = value;
			NSColor* fixedColor = [self correctColor: origColor];
			if ( fixedColor != nil )
			{
				[attStr
					addAttribute: NSForegroundColorAttributeName
					value: fixedColor
					range: range];
			}
		}];

	[attStr enumerateAttribute: NSUnderlineColorAttributeName
				inRange: NSMakeRange( 0, attStr.length )
				options: 0
				usingBlock:
		^(id  _Nullable value, NSRange range, BOOL * _Nonnull stop)
		{
			NSColor* origColor = value;
			NSColor* fixedColor = [self correctColor: origColor];
			if ( fixedColor != nil )
			{
				[attStr
					addAttribute: NSUnderlineColorAttributeName
					value: fixedColor
					range: range];
			}
		}];
}

- (NSMutableAttributedString*) loadTextFromDict: (NSDictionary*) dict
								error: (NSError* _Nullable * _Nonnull ) outError
{
	NSMutableAttributedString* resultStr = nil;
	
	// Newer versions of PlainCalc save the text using NSKeyedArchiver,
	// ensuring that text color information is preserved.
	NSData* textArchive = dict[@"textArchive"];
	if (textArchive != nil)
	{
		resultStr = [NSKeyedUnarchiver
			unarchivedObjectOfClass: NSMutableAttributedString.class
			fromData: textArchive
			error: outError];
		if (resultStr == nil)
		{
			NSLog(@"NSKeyedUnarchiver error %@", *outError );
		}
	}
	else // reading file created by older version of program
	{
		NSString*	theRTFStr = dict[@"text"];
		if (theRTFStr != nil)
		{
			NSData*	theRTFdata = [theRTFStr
				dataUsingEncoding: NSUTF8StringEncoding];
			if (theRTFdata != nil)
			{
				NSDictionary<NSAttributedStringDocumentReadingOptionKey, id>*
					readOptions = @{
						NSDocumentTypeDocumentAttribute: NSRTFTextDocumentType
					};
				resultStr = [[NSMutableAttributedString alloc]
					initWithData: theRTFdata
					options: readOptions
					documentAttributes: nil
					error: outError];
				if (resultStr != nil)
				{
					[self correctTextColors: resultStr];
				}
			}
		}
	}
	
	return resultStr;
}

- (NSError*) loadNativeData: (NSData*) data
{
	NSError* err = nil;
	NSDictionary*	theDict = [NSPropertyListSerialization
		propertyListWithData: data
		options: NSPropertyListImmutable
		format: NULL
		error: &err ];
		
	if (theDict != nil)
	{
		NSMutableAttributedString*	theAttStr = [self
			loadTextFromDict: theDict
			error: &err];
		
		if (theAttStr != nil)
		{
			if (_textView == nil)
			{
				// We will not have loaded the nib yet, so we must save
				// the text in a member variable instead of putting it
				// in the text view.
				_initialText = theAttStr;
			}
			else
			{
				[_textView.textStorage setAttributedString: theAttStr];
			}
		}
		
		// Restore the typing font that was last in use
		NSString* fontName = theDict[@"fontName"];
		NSNumber* fontSize = theDict[@"fontSize"];
		if ( (fontName != nil) and (fontSize != nil) )
		{
			NSFontManager*	fontMgr = [NSFontManager sharedFontManager];
			NSFont* typingFont = [fontMgr
				fontWithFamily: fontName
				traits: 0
				weight: 5
				size: fontSize.floatValue ];
			if (typingFont != nil)
			{
				_initialTypingFont = typingFont;
			}
		}
		
		// Restore the variables and functions
		LoadStateFromDictionary( _calcState, theDict );
		
		// Restore window frame
		_initialWindowFrame = theDict[@"windowFrame"];
	}
	
	return err;
}

//MARK: calculation

- (void) showOverlay
{
	_calculatingOverlay.hidden = NO;
}

- (void) restoreEditability
{
	[NSObject cancelPreviousPerformRequestsWithTarget: self];
	[_textView setEditable: YES];
	_calculatingOverlay.hidden = YES;
}

- (void) showAnswer: (NSNumber*) result
{
	[self restoreEditability];
	double value = result.doubleValue;
	[self insertString: [self formatCalculatedResult: value]
			withAttributes: AppDelegate.successTextAtts ];
	
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (void) sayDefinedFunc: (NSString*) funcName
{
	[self restoreEditability];
	NSString* msgFormat = NSLocalizedString( @"DefFun", nil );
	NSString* msg = [NSString stringWithFormat: msgFormat, funcName ];
	[self insertString: msg
			withAttributes: AppDelegate.successTextAtts ];
	
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (void) sayRedefinedFunc: (NSString*) funcName
{
	[self restoreEditability];
	NSString* msgFormat = NSLocalizedString( @"ReDefFun", nil );
	NSString* msg = [NSString stringWithFormat: msgFormat, funcName ];
	[self insertString: msg
			withAttributes: AppDelegate.successTextAtts ];
	
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (void) reportParseError: (NSString*) errorStr
{
	[self restoreEditability];
	
	NSRange firstColon = [errorStr rangeOfString: @":"];
	if (firstColon.location != NSNotFound)
	{
		NSRange pastFirstColon = NSMakeRange( firstColon.location+1,
			errorStr.length - firstColon.location - 1 );
		NSRange secondColon = [errorStr rangeOfString: @": "
				options: NSLiteralSearch
				range: pastFirstColon ];
		if (secondColon.location != NSNotFound)
		{
			NSRange errorPlaceRange = NSMakeRange( firstColon.location+1,
				secondColon.location - firstColon.location - 1 );
			if (errorPlaceRange.length > 0)
			{
				NSString* errorPlaceStr = [errorStr substringWithRange: errorPlaceRange];
				NSInteger errorPlace = errorPlaceStr.integerValue;
				NSRange pastSecondColon = NSMakeRange( secondColon.location + 2,
					errorStr.length - secondColon.location - 2 );
				NSRange thirdColon = [errorStr rangeOfString: @":\n"
					options: NSLiteralSearch
					range: pastSecondColon];
				if (thirdColon.location != NSNotFound)
				{
					NSRange mainTextRange = NSMakeRange( secondColon.location + 2,
						thirdColon.location - secondColon.location - 2 );
					errorStr = [errorStr substringWithRange: mainTextRange];
				}
				
				// The value errorPlace given by the parser is, I think, a
				// number of UTF-32 code points.  We must convert that to a
				// number of UTF-16 code points.
				NSString* calculatedText = [_textView.textStorage.string
					substringWithRange: _lastCalculatedLineRange];
				std::u32string line32( UTF8toUTF32( calculatedText.UTF8String ) );
				if (errorPlace < line32.length())
				{
					line32.erase( errorPlace );
					std::u16string line16( UTF32toUTF16( line32 ) );
					errorPlace = _lastCalculatedLineRange.location + line16.length();
					if (NSLocationInRange( errorPlace, _lastCalculatedLineRange ))
					{
						NSRange badRange = NSMakeRange(
							errorPlace,
							NSMaxRange(_lastCalculatedLineRange) - errorPlace);
						NSDictionary* badTextAtts = @{
							NSUnderlineColorAttributeName: [NSColor colorNamed: @"BadResult"],
							NSUnderlineStyleAttributeName: @(NSUnderlineStyleThick)
						};
						[_textView.textStorage addAttributes: badTextAtts range: badRange];
					}
				}
			}
		}
	}
	else
	{
		errorStr = NSLocalizedString( @"WeirdErr", nil );
	}
	
	[self insertString: errorStr
			withAttributes: AppDelegate.errorTextAtts ];
	
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (void) reportInterrupt: (NSNumber*) interruptCode
{
	[self restoreEditability];
	CalcInterruptCode code = static_cast<CalcInterruptCode>( interruptCode.intValue );
	if (code == CalcInterruptCode::userAbort)
	{
		[self insertString: NSLocalizedString( @"UserCancel", nil )
			withAttributes: AppDelegate.errorTextAtts ];
	}
	else if (code == CalcInterruptCode::stackLimit)
	{
		[self insertString: NSLocalizedString( @"StackLimit", nil )
			withAttributes: AppDelegate.errorTextAtts ];
	}
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (void) calculateLine: (NSString*) theLine
{
	__weak Document* weakSelf = self;
	
	PerformBlockOnWorkThread(
		^{
			Document* me = weakSelf;
			if (me == nil)
			{
				return;
			}
			
			CalcResult result = Calculate( theLine.UTF8String, me->_calcState );
			
			if (std::holds_alternative<double>( result ))
			{
				double answer = std::get<double>( result );
				[me performSelectorOnMainThread: @selector(showAnswer:)
					withObject: @(answer)
					waitUntilDone: NO];
			}
			else if (std::holds_alternative<DefinedFunc>( result ))
			{
				DefinedFunc funcName = std::get<DefinedFunc>( result );
				if (funcName.redefined)
				{
					[me performSelectorOnMainThread: @selector(sayRedefinedFunc:)
						withObject: @(funcName.name.c_str())
						waitUntilDone: NO];
				}
				else
				{
					[me performSelectorOnMainThread: @selector(sayDefinedFunc:)
						withObject: @(funcName.name.c_str())
						waitUntilDone: NO];
				}
			}
			else if (std::holds_alternative<std::string>( result ))
			{
				std::string errorStr = std::get<std::string>( result );
				[me performSelectorOnMainThread: @selector(reportParseError:)
					withObject: @(errorStr.c_str())
					waitUntilDone: NO];
			}
			else if (std::holds_alternative<CalcInterruptCode>( result ))
			{
				CalcInterruptCode code = std::get<CalcInterruptCode>( result );
				[me performSelectorOnMainThread: @selector(reportInterrupt:)
					withObject: @(static_cast<int>(code))
					waitUntilDone: NO];
			}
		});
}


//MARK: NSDocument overloads

- (NSString *)windowNibName
{
	return @"Document";
}

- (void)windowControllerDidLoadNib:(NSWindowController *)windowController
{
	assert( _textView != nil );

	if (_initialText == nil) // new document
	{
		NSData* theData = [NSData dataWithContentsOfURL: [self newDocStateURL]];
		if (theData != nil)
		{
			NSError* err = [self loadNativeData: theData];
			if (err == nil)
			{
				if (_initialText != nil) // to make static analyzer happy
				{
					[_textView.textStorage setAttributedString: _initialText];
				}
			}
			else
			{
				_textView.textColor = NSColor.textColor;
				_textView.font = _initialTypingFont;
			}
		}
	}
	else // old document
	{
		[_textView.textStorage setAttributedString: _initialText];
	}
	_initialText = nil;

	// Set typing attributes
	NSMutableDictionary* typingAtts = [NSMutableDictionary
		dictionaryWithDictionary: _textView.typingAttributes ];
	[typingAtts setValue: _initialTypingFont
				forKey: NSFontAttributeName];
	[typingAtts setValue: NSColor.textColor
				forKey: NSForegroundColorAttributeName];
	_textView.typingAttributes = typingAtts;
	
	if (_initialWindowFrame)
	{
		[_docWindow setFrameFromString: _initialWindowFrame];
		_initialWindowFrame = nil;
	}
}


- (NSData *)dataOfType: (NSString *)typeName error: (NSError **)outError
{
	NSError* theError = nil;
	NSData* textArchive = [NSKeyedArchiver
		archivedDataWithRootObject: _textView.textStorage
		requiringSecureCoding: YES
		error: &theError];
	if (textArchive == nil)
	{
		NSLog( @"NSKeyedArchiver Error: %@", theError );
		if (outError)
		{
			*outError = theError;
		}
		return nil;
	}
	
	NSFont* typingFont = [_textView.typingAttributes valueForKey: NSFontAttributeName];
	
	NSArray<NSDictionary*>* userDefs = SaveStateToDictionary( _calcState );
	NSDictionary* variables = userDefs[0];
	NSDictionary* funcV2 = userDefs[1];
	NSDictionary* funcV3 = userDefs[2];

	NSDictionary*	docDict = @{
		@"textArchive": textArchive,
		@"variables": variables,
		@"functions": funcV2,
		@"functions_v3": funcV3,
		@"fontName": [typingFont familyName],
		@"fontSize": @([typingFont pointSize]),
		@"windowFrame": [_docWindow stringWithSavedFrame]
	};
	
	NSData*	data = [NSPropertyListSerialization
		dataWithPropertyList: docDict
		format: NSPropertyListXMLFormat_v1_0
		options: 0
		error: &theError ];
	if (theError != nil)
	{
		NSLog( @"NSPropertyListSerialization Error: %@", theError );
		
		if (outError)
		{
			*outError = theError;
		}
	}

    return data;
}


- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
	NSError* loadError = nil;
	if ( [typeName isEqualToString: @"com.jwwalker.plaincalc"] or
		[typeName isEqualToString: @"PlainCalc worksheet"] )
	{
		loadError = [self loadNativeData: data];
	}
	else
	{
		[NSException raise:@"UnimplementedMethod" format:@"%@ is unimplemented", NSStringFromSelector(_cmd)];
	}
	// Insert code here to read your document from the given data of the specified type. If outError != NULL, ensure that you create and set an appropriate error if you return NO.
	// Alternatively, you could remove this method and override -readFromFileWrapper:ofType:error: or -readFromURL:ofType:error: instead.
	// If you do, you should also override -isEntireFileLoaded to return NO if the contents are lazily loaded.
	return (loadError == nil);
}

- (void)canCloseDocumentWithDelegate:(id)delegate
		shouldCloseSelector:(SEL)shouldCloseSelector
		contextInfo:(void *)contextInfo;
{
	// The point of this song and dance is to allow the user to close the
	// window without being asked to save it, if it has never been saved
	// to a file.
	if ( (not [self isDocumentEdited]) or
		([self fileURL] == nil) )
	{
		NSMethodSignature* theSig = [delegate methodSignatureForSelector:
			shouldCloseSelector];
		NSInvocation* invoc = [NSInvocation invocationWithMethodSignature: theSig];
		[invoc setSelector: shouldCloseSelector];
		[invoc setTarget: delegate];
		[invoc setArgument: (void*)&self atIndex: 2];
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

- (NSPrintInfo *)printInfo
{
    NSPrintInfo *printInfo = [super printInfo];
	[printInfo setHorizontalPagination: NSPrintingPaginationModeFit];
	[printInfo setHorizontallyCentered: NO];
	[printInfo setVerticallyCentered: NO];
	[printInfo setLeftMargin: 72.0];
	[printInfo setRightMargin: 72.0];
	[printInfo setTopMargin: 72.0];
	[printInfo setBottomMargin: 72.0];
    return printInfo;
}

- (IBAction) doPrint: (id) sender
{
	// Make an offscreen NSTextView to print
	NSRect printFrame = {
		{ 0.0, 0.0 },
		{ 468.0, 100.0 }
	};
	NSTextView* printView = [[NSTextView alloc] initWithFrame: printFrame];
	[printView setHorizontallyResizable: NO];
	[printView setVerticallyResizable: YES];
	[printView.textStorage setAttributedString: _textView.textStorage];
	
	NSPrintOperation *op = [NSPrintOperation
                printOperationWithView: printView ];
	op.jobTitle = self.displayName;
	op.showsPrintPanel = YES;
	op.showsProgressPanel = YES;
	op.printInfo.verticallyCentered = NO;
	op.printInfo.horizontallyCentered = NO;
	[op.printInfo setLeftMargin: 72.0];
	[op.printInfo setRightMargin: 72.0];
	[op.printInfo setTopMargin: 72.0];
	[op.printInfo setBottomMargin: 72.0];
	
	NSPrintPanel* panel = op.printPanel;
	panel.options |= NSPrintPanelShowsCopies | NSPrintPanelShowsPageRange |
		NSPrintPanelShowsPaperSize | NSPrintPanelShowsOrientation |
		NSPrintPanelShowsPreview;

	[op runOperationModalForWindow: _textView.window
                delegate: nil
                didRunSelector:  nil
                contextInfo: nil];
}

//MARK: localization

/// Take a string with the usual period as decimal point and comma as parameter
/// separactor, and change it to use the localized decimal point.
- (NSString*) localizeString: (NSString*) stdString
{
	NSString* locString = stdString;
	
	if ( [AppDelegate isCommaDecimalSeparator] )
	{
		NSString* workStr = [stdString stringByReplacingOccurrencesOfString: @","
										withString: @";"];
		locString = [workStr stringByReplacingOccurrencesOfString: @"."
										withString: @","];
	}
	
	return locString;
}


/// Take a string that may use a comma as a decimal separator, and convert it to
/// one with the usual period as decimal separator.
- (NSString*) standardizeString: (NSString*) locString
{
	NSString* stdString = locString;
	
	if ( [AppDelegate isCommaDecimalSeparator] )
	{
		NSString* workStr = [locString stringByReplacingOccurrencesOfString: @","
										withString: @"."];
		stdString = [workStr stringByReplacingOccurrencesOfString: @";"
							withString: @","];
	}
	
	return stdString;
}

//MARK: NSWindowDelegate

- (void)windowWillClose:(NSNotification *)notification
{
	// If a calculation is in progress, ask it to stop, because a calculation
	// block holds a strong reference to the document.
	_calcState.interruptCode = CalcInterruptCode::userAbort;
}

//MARK: NSText delegate

- (void) textDidChange: (NSNotification *) notification
{
	[self updateChangeCount: NSChangeDone];
}


//MARK: NSTextViewDelegate

- (BOOL) handleEnterKey
{
	BOOL	didHandle = NO;

	NSRange	theRange = [_textView selectedRange];
	if ( (theRange.length == 0) and (theRange.location > 0) )
	{
		// Find the previous linefeed, and then the start of this line
		NSString*	thePlainText = _textView.textStorage.string;
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
		if (lineEnd > lineStart) // nonempty line
		{
			NSRange	lineRange = NSMakeRange( lineStart, lineEnd - lineStart );
			_lastCalculatedLineRange = lineRange;
			NSString*	theLine = [thePlainText
				substringWithRange: lineRange];
			
			// Anything from a '#' onward is a comment, and ignored
			NSRange poundRange = [theLine rangeOfString: @"#"];
			if (poundRange.location != NSNotFound)
			{
				theLine = [theLine substringToIndex: poundRange.location];
			}
			
			// Trim trailing white space
			NSRange whiteEnd = [theLine
				rangeOfCharacterFromSet: NSCharacterSet.whitespaceCharacterSet
				options: NSBackwardsSearch | NSAnchoredSearch];
			if (whiteEnd.length > 0)
			{
				theLine = [theLine substringToIndex: whiteEnd.location];
			}
				
			// de-localize
			theLine = [self standardizeString: theLine];
			
			if (theLine.length > 0)
			{
				// insert = and then line break
				[_textView insertText: @" =\n"
					replacementRange: _textView.selectedRange];
				
				// Do not let the user edit while we are calculating
				[_textView setEditable: NO];
				[self performSelector: @selector(showOverlay)
						withObject: nil
						afterDelay: 0.5];
				
				[self calculateLine: theLine];
				
				didHandle = YES;
			}
		}
	}
	
	return didHandle;
}

- (BOOL) textView:(NSTextView *)aTextView doCommandBySelector: (SEL)aSelector
{
	BOOL	didHandle = NO;
	
	// When the user presses Enter or Return at the end of a line, we usually
	// want to do a calculation with it.
	if ( (aTextView == _textView) and (aSelector == @selector(insertNewline:)) )
	{
		didHandle = [self handleEnterKey];
	}
	else if ( (aTextView == _textView) and (aSelector == @selector(cancelOperation:)) )
	{
		_calcState.interruptCode = CalcInterruptCode::userAbort;
		didHandle = YES;
	}
	
	return didHandle;
}

- (NSDictionary *)textView:(NSTextView *)textView
				shouldChangeTypingAttributes:(NSDictionary *) oldTypingAttributes
				toAttributes:(NSDictionary *)newTypingAttributes
{
	NSDictionary * attsToUse = newTypingAttributes;
	
	NSColor* colorAtt = [attsToUse valueForKey: NSForegroundColorAttributeName];
	
	if ( (colorAtt != nil) and (not [colorAtt isEqual: NSColor.textColor]) )
	{
		attsToUse = [NSMutableDictionary dictionaryWithDictionary: attsToUse];
		[attsToUse setValue: NSColor.textColor
					forKey: NSForegroundColorAttributeName];
	}
	
	return attsToUse;
}

//MARK: NSMenuItemValidation

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	BOOL isEnabled = YES;
	if (menuItem.action == @selector(formatIntegersAsDecimal:))
	{
		menuItem.state = _formatIntegersAsHex? NSControlStateValueOff :
			NSControlStateValueOn;
	}
	else if (menuItem.action == @selector(formatIntegersAsHex:))
	{
		menuItem.state = _formatIntegersAsHex? NSControlStateValueOn :
			NSControlStateValueOff;
	}
	else if (menuItem.action == @selector(revertDocumentToSaved:))
	{
		isEnabled = self.isDocumentEdited and (self.fileURL != nil);
	}
	else if (menuItem.action == @selector(saveDocument:))
	{
		isEnabled = self.isDocumentEdited or (self.fileURL == nil);
	}
	return isEnabled;
}

//MARK: Actions



- (IBAction) formatIntegersAsDecimal:(id)sender
{
	_formatIntegersAsHex = NO;
}

- (IBAction) formatIntegersAsHex:(id)sender
{
	_formatIntegersAsHex = YES;
}

- (IBAction) showVariables:(id)sender
{
	NSMutableString* theStr = [[NSMutableString alloc] initWithCapacity:100];
	[theStr appendString: NSLocalizedString( @"DefVars", nil ) ];

	for (const auto& [key, value] : _calcState.variables)
	{
		NSString*	valueStr = [self formatCalculatedResult: value];
		[theStr appendFormat: @"\n%@ = %@", @(key.c_str()), valueStr ];
	}
	
	[self insertString: theStr
		withAttributes: AppDelegate.successTextAtts ];
	
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (IBAction) showFunctions:(id)sender
{
	NSMutableString* theStr = [[NSMutableString alloc] initWithCapacity:100];
	[theStr appendString: NSLocalizedString( @"DefFuns", nil ) ];
	
	NSString* fmtWithSep = AppDelegate.isCommaDecimalSeparator?
		@"; %@" : @", %@";
	
	for (const auto& [name, value] : _calcState.userFunctions)
	{
		const StringVec& params( std::get<StringVec>( value ) );
		const std::string rhs( std::get<std::string>( value ) );
		[theStr appendFormat: @"\n%@( ", @(name.c_str()) ];
		BOOL isFirst = YES;
		for (const std::string& param : params)
		{
			if (isFirst)
			{
				[theStr appendString: @(param.c_str()) ];
				isFirst = NO;
			}
			else
			{
				[theStr appendFormat: fmtWithSep, @(param.c_str()) ];
			}
		}
		[theStr appendFormat: @" ) = %@", @(rhs.c_str()) ];
	}
	
	[self insertString: theStr
		withAttributes: AppDelegate.successTextAtts ];
	
	[self insertString: @"\n"
		withAttributes: AppDelegate.normalTextAtts ];
}

- (IBAction) sheetOK:(id)sender
{
	[_docWindow endSheet: _deleteSymbolSheet returnCode: NSModalResponseOK];
}

- (IBAction) sheetCancel:(id)sender
{
	[_docWindow endSheet: _deleteSymbolSheet returnCode: NSModalResponseCancel];
}

- (IBAction) forgetVariable:(id)sender
{
	[_deleteSymbolPopup removeAllItems];
	for (const auto& [name, value] : _calcState.variables)
	{
		[_deleteSymbolPopup addItemWithTitle: @(name.c_str())];
	}
	_deleteSymbolOKButton.enabled = (_deleteSymbolPopup.numberOfItems > 0);
	
	[_docWindow beginSheet: _deleteSymbolSheet
				completionHandler:
		^(NSModalResponse returnCode)
		{
			if (returnCode == NSModalResponseOK)
			{
				NSString* symbolName = [self->_deleteSymbolPopup titleOfSelectedItem];
				self->_calcState.variables.erase( symbolName.UTF8String );
			}
		}];
}

- (IBAction) forgetFunction:(id)sender
{
	[_deleteSymbolPopup removeAllItems];
	for (const auto& [name, value] : _calcState.userFunctions)
	{
		[_deleteSymbolPopup addItemWithTitle: @(name.c_str())];
	}
	_deleteSymbolOKButton.enabled = (_deleteSymbolPopup.numberOfItems > 0);
	
	[_docWindow beginSheet: _deleteSymbolSheet
				completionHandler:
		^(NSModalResponse returnCode)
		{
			if (returnCode == NSModalResponseOK)
			{
				NSString* symbolName = [self->_deleteSymbolPopup titleOfSelectedItem];
				self->_calcState.userFunctions.erase( symbolName.UTF8String );
			}
		}];
}

- (IBAction) saveAsNewDocumentContent: (id) sender
{
	NSAlert* theAlert = [[NSAlert alloc] init];
	[theAlert setMessageText: NSLocalizedString( @"ConfirmSaveNew", nil )];
	[theAlert setInformativeText: NSLocalizedString( @"ConfirmSaveNew_detail", nil )];
	[theAlert addButtonWithTitle: NSLocalizedString( @"ConfirmSaveNew_ok", nil )];
	[theAlert addButtonWithTitle: NSLocalizedString( @"ConfirmSaveNew_cancel", nil )];
	__weak NSWindow* weakWindow = _docWindow;
	
	[theAlert beginSheetModalForWindow: _docWindow
		completionHandler: ^(NSModalResponse returnCode)
		{
			if (returnCode == NSAlertFirstButtonReturn)
			{
				[[theAlert window] orderOut: theAlert];
				
				NSData* theData = [self dataOfType: @"com.jwwalker.plaincalc" error: nil];
				
				if (theData)
				{
					NSError* theErr = nil;
					
					if (not [theData writeToURL: [self newDocStateURL]
									options: NSDataWritingAtomic
									error: &theErr ])
					{
						NSAlert* errAlert = [NSAlert alertWithError: theErr];
						[errAlert beginSheetModalForWindow: weakWindow
									completionHandler: nil];
					}
				}
			}
		}];
}


@end

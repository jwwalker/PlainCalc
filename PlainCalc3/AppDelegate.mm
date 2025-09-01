//  AppDelegate.m
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

#import "AppDelegate.h"

#import "HelpWindowController.h"
#import "PerformBlockOnWorkThread.h"

#import <pthread.h>

static AppDelegate* sMe = nil;

@interface AppDelegate ()

@end

@implementation AppDelegate
{
	NSDictionary*	_normalAtts;
	NSDictionary*	_successAtts;
	NSDictionary*	_errorAtts;
	BOOL			_isCommaDecimalSeparator;
	HelpWindowController*	_helpWindow;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	sMe = self;
	
	_normalAtts = @{
		NSForegroundColorAttributeName: NSColor.textColor
	};
	_successAtts = @{
		NSForegroundColorAttributeName: [NSColor colorNamed: @"GoodResult" ]
	};
	_errorAtts = @{
		NSForegroundColorAttributeName: [NSColor colorNamed: @"BadResult" ]
	};
	
	NSString* decimalSep = [NSLocale.currentLocale objectForKey:
			NSLocaleDecimalSeparator ];
	_isCommaDecimalSeparator = [decimalSep isEqualToString: @","];

	[NSNotificationCenter.defaultCenter
		addObserver: self
		selector: @selector(localeChanged:)
		name: NSCurrentLocaleDidChangeNotification
		object: nil];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
	// Insert code here to tear down your application
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
{
	return YES;
}

- (void) localeChanged: (NSNotification*) notification
{
	NSString* decimalSep = [NSLocale.currentLocale objectForKey:
		NSLocaleDecimalSeparator ];
	_isCommaDecimalSeparator = [decimalSep isEqualToString: @","];
	//NSLog(@"Decimal separator is %@", mCommaIsDecimal? @"comma" : @"period");
}

- (void) reportInternetError: (NSError*) error
{
	NSString* title = NSLocalizedString( @"NoMathErr", nil );
	NSString* msgFmt = NSLocalizedString( @"NoMathErr_d", nil );
	NSString* msg = [NSString stringWithFormat: msgFmt, error.localizedDescription];
	NSAlert* alert = [[NSAlert alloc] init];
	alert.messageText = title;
	alert.informativeText = msg;
	[alert beginSheetModalForWindow: _helpWindow.window
			completionHandler: nil];
}

- (IBAction) showHelp: (id) sender
{
	_helpWindow = [[HelpWindowController alloc]
		initWithWindowNibName: @"Help"];
	_helpWindow.window.delegate = self;
	[_helpWindow showWindow: self];
	
	// Check whether we can get MathJAX over the Internet
	NSURL* mathURL = [NSURL URLWithString:
		@"https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.6/MathJax.js"];
	NSURLRequest* req = [NSURLRequest
		requestWithURL: mathURL
		cachePolicy: NSURLRequestReloadIgnoringLocalCacheData
		timeoutInterval: 5.0];
	NSURLSessionDataTask* task = [NSURLSession.sharedSession
		dataTaskWithRequest: req
		completionHandler:
			^(NSData * _Nullable data, NSURLResponse * _Nullable response,
				NSError * _Nullable error)
			{
				if (error != nil)
				{
					[self performSelectorOnMainThread: @selector(reportInternetError:)
						withObject: error
						waitUntilDone: NO];
				}
				else
				{
					NSLog(@"Found the MathJax resource");
				}
			}];
	[task resume];
}

//MARK: NSWindowDelegate

- (void)windowWillClose:(NSNotification *)notification
{
	NSLog(@"Help window will close");
	_helpWindow = nil;
}

//MARK: class methods

+ (NSDictionary*) normalTextAtts
{
	return sMe->_normalAtts;
}

+ (NSDictionary*) successTextAtts
{
	return sMe->_successAtts;
}

+ (NSDictionary*) errorTextAtts
{
	return sMe->_errorAtts;
}

+ (BOOL) isCommaDecimalSeparator
{
	return sMe->_isCommaDecimalSeparator;
}

@end

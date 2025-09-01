//  HelpWindowController.m
//  PlainCalc3
//
//  Created by James Walker on 8/18/25.
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

#import "HelpWindowController.h"

#import <WebKit/WebKit.h>

@interface HelpWindowController ()
{
	IBOutlet WKWebView*		_webView;
	WKWebView*				_printView;
}

@end

@implementation HelpWindowController

- (void) dealloc
{
	NSLog(@"HelpWindowController dealloc");
}

- (void) windowDidLoad
{
    [super windowDidLoad];
    
    NSURL* helpURL = [NSBundle.mainBundle
		URLForResource: @"PlainCalcHelp.md"
		withExtension: @"html"
		subdirectory: @"Help"];
	
	[_webView loadFileURL: helpURL
		allowingReadAccessToURL: helpURL.URLByDeletingLastPathComponent ];
}

- (void) printMe
{
	NSPrintInfo* info = NSPrintInfo.sharedPrintInfo;
	info.topMargin = 72.0;
	info.bottomMargin = 72.0;
	info.leftMargin = 72.0;
	info.rightMargin = 72.0;
	info.verticallyCentered = NO;
	info.horizontallyCentered = NO;
	
	NSPrintOperation* printOp = [_printView printOperationWithPrintInfo: info];
	printOp.jobTitle = @"PlainCalc Help";

	// Note: when running under the debugger, I get a debugger break saying
	// "ERROR: The NSPrintOperation view's frame was not initialized properly
	// before knowsPageRange: returned."  But if I just continue, printing
	// seems to work fine.
	
	[printOp runOperationModalForWindow: _webView.window
			delegate: self
			didRunSelector: nil
			contextInfo: nil ];
	
	_printView = nil;
}

// Navigation delegate, only used for printing
- (void)webView: (WKWebView *)webView 
	didFinishNavigation:(WKNavigation *)navigation
{
	if (webView == _printView)
	{
		[self performSelector: @selector(printMe) withObject: nil afterDelay: 0.1];
	}
}

- (void)webView:(WKWebView *)webView 
	didFailProvisionalNavigation:(WKNavigation *)navigation 
      withError:(NSError *)error
{
	NSLog(@"didFailProvisionalNavigation");
}

- (void)webView:(WKWebView *)webView 
	didFailNavigation:(WKNavigation *)navigation 
      withError:(NSError *)error
{
	NSLog(@"didFailNavigation");
}

- (void)webViewWebContentProcessDidTerminate:(WKWebView *)webView
{
	NSLog(@"webViewWebContentProcessDidTerminate");
}

- (IBAction) doPrint: (id) sender
{
	NSRect printFrame = {
		{ 0.0, 0.0 },
		{ 468.0, 100.0 }
	};
	_printView = [[WKWebView alloc] initWithFrame: printFrame];
	_printView.navigationDelegate = self;
    NSURL* helpURL = [NSBundle.mainBundle
		URLForResource: @"PlainCalcHelp.md"
		withExtension: @"html"
		subdirectory: @"Help"];
	[_printView loadFileURL: helpURL
		allowingReadAccessToURL: helpURL.URLByDeletingLastPathComponent ];
	// If/when the load finishes, my webView:didFinishNavigation: delegate
	// method should be called.
}

@end

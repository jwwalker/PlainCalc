//  HelpWindow.swift
//  PlainCalc3
//
//  Created by James Walker on 9/25/25.
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

import Foundation
import AppKit
import WebKit

@objc
class HelpWindow: NSWindowController, WKNavigationDelegate
{
	@IBOutlet var webView: WKWebView!
	var printView: WKWebView?
	
	deinit
	{
		print( "HelpWindow dealloc" )
	}
	
	override func windowDidLoad()
	{
		if let helpURL = Bundle.main.url( forResource: "PlainCalcHelp.md",
			withExtension: "html", subdirectory: "Help" )
		{
			webView.loadFileURL( helpURL,
				allowingReadAccessTo: helpURL.deletingLastPathComponent() )
		}
	}
	
	func printMe()
	{
		let info = NSPrintInfo.shared
		info.topMargin = 72.0;
		info.bottomMargin = 72.0;
		info.leftMargin = 72.0;
		info.rightMargin = 72.0;
		info.isVerticallyCentered = false;
		info.isHorizontallyCentered = false;
		
		// Note: when running under the debugger, I get a debugger break saying
		// "ERROR: The NSPrintOperation view's frame was not initialized properly
		// before knowsPageRange: returned."  But if I just continue, printing
		// seems to work fine.
		let printOp = printView!.printOperation( with: info );
		printOp.jobTitle = "PlainCalc Help";
		
		printOp.runModal(for: webView.window!, delegate: self,
			didRun: nil, contextInfo: nil )
		
		printView = nil
	}
	
	func webView(
		_ inWebView: WKWebView,
		didFinish navigation: WKNavigation! )
	{
		if (inWebView == printView) && (webView != nil)
		{
			self.printMe()
		}
	}
	
	func webView(
		_ webView: WKWebView,
		decidePolicyFor navigationAction: WKNavigationAction,
		decisionHandler: @escaping @MainActor (WKNavigationActionPolicy) -> Void )
	{
		var decided = false
		
		if (navigationAction.navigationType == WKNavigationType.linkActivated)
		{
			if let url = navigationAction.request.url
			{
				if url.scheme == "file"
				{
					// navigation link within the help file
					decisionHandler( WKNavigationActionPolicy.allow )
				}
				else
				{
					// Do not handle external links here; let https links
					// be handled by the default web browser, and let
					// mailto links be handled by Mail.
					decisionHandler( WKNavigationActionPolicy.cancel )
					NSWorkspace.shared.open( url )
				}
				decided = true
			}
		}
		
		if !decided
		{
			decisionHandler( WKNavigationActionPolicy.allow )
		}
	}
	

	@IBAction func doPrint(_ senderr: Any)
	{
		let printFrame = NSMakeRect( 0.0, 0.0, 468.0, 100.0 )
		printView = WKWebView( frame: printFrame )
		if let printView
		{
			printView.navigationDelegate = self
			if let helpURL = Bundle.main.url( forResource: "PlainCalcHelp.md",
				withExtension: "html", subdirectory: "Help" )
			{
				printView.loadFileURL( helpURL,
					allowingReadAccessTo: helpURL.deletingLastPathComponent() )
				// If/when the load finishes, my webView:didFinishNavigation:
				// delegate method should be called.
			}
		}
	}
}

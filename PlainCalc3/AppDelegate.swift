//  AppDelegate.swift
//  PlainCalc3
//
//  Created by James Walker on 9/29/25.
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


@MainActor @objcMembers
class AppDelegate: NSObject, NSApplicationDelegate, NSWindowDelegate
{
	static let normalTextAtts = [NSAttributedString.Key.foregroundColor: NSColor.textColor];
	static let successTextAtts = [NSAttributedString.Key.foregroundColor: NSColor(named: "GoodResult")!];
	static let errorTextAtts = [NSAttributedString.Key.foregroundColor: NSColor(named: "BadResult")!];
	
	static var isCommaDecimalSeparator: Bool {
		var result = false
		if let sep = Locale.current.decimalSeparator
		{
			result = sep == ","
		}
		return result
	}
	
	var helpWindow : HelpWindow?
	
	func reportInternetError( _ error: Error )
	{
		let title = NSLocalizedString("NoMathErr", comment: "title of internet loading error message")
		let msgFmt = NSLocalizedString("NoMathErr_d", comment: "detail of internet loading error message")
		let msg = String(format: msgFmt, error.localizedDescription )
		let alert = NSAlert()
		alert.messageText = title;
		alert.informativeText = msg;
		if let window = helpWindow?.window
		{
			alert.beginSheetModal(for: window )
		}
	}
	
	func checkForMathJAX()
	{
		if let testURL = URL( string: "https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.6/MathJax.js" )
		{
			let request = URLRequest( url: testURL,
									cachePolicy: .reloadIgnoringLocalCacheData,
									timeoutInterval: 5.0 )
			
			let task = URLSession.shared.dataTask(with: request,
						completionHandler:
						{(_: Data?, _: URLResponse?, error: Error?) in
							if let error
							{
								DispatchQueue.main.async
								{
									self.reportInternetError( error )
								}
							}
						})
			task.resume()
		}
	}
	
	//MARK: NSApplicationDelegate

	@MainActor
	func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool
	{
		return true
	}
	
	//MARK: NSWindowDelegate
	@MainActor
	func windowWillClose(_ notification: Notification)
	{
		helpWindow = nil
	}
	
	//MARK: Actions
	@IBAction func showHelp(_ senderr: Any)
	{
		helpWindow = HelpWindow( windowNibName: "Help" )
		if let theWindow = helpWindow?.window
		{
			theWindow.delegate = self;
			helpWindow?.showWindow( self )
			
			checkForMathJAX()
		}
	}
}

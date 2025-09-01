//  PerformBlockOnWorkThread.m
//  PlainCalc3
//
//  Created by James Walker on 8/17/25.
//  
//
/*
	Copyright (c) 2006-2025 James W. Walker

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

#import "PerformBlockOnWorkThread.h"

#import <Cocoa/Cocoa.h>

static constexpr size_t	kWorkerStackSize = 2U * 1024U * 1024U; 	// 2 megabytes

static void DoNothingRunLoopCallback( void* info )
{
	
}

@interface Performer : NSObject

- (void) runThread: (id) dummy;

- (void) doBlock: (WorkBlock) block;

@end

static Performer* MakePerformer()
{
	static Performer* performer = [[Performer alloc] init];
	return performer;
}

static NSThread* MakeThread( Performer* performer )
{
	NSThread* thread = [[NSThread alloc]
		initWithTarget: performer
		selector: @selector(runThread:)
		object: nil];
	
	thread.stackSize = kWorkerStackSize;
	thread.name = @"BlockPerformer";
	
	[thread start];
	
	return thread;
}

@implementation Performer

- (void) runThread: (id) dummy
{
	// See https://shaheengandhi.com/controlling-thread-exit/
	// for discussion of how this thread is set up.
	@autoreleasepool
	{
		CFRunLoopSourceContext context = { 0 };
		context.perform = DoNothingRunLoopCallback;
		
		CFRunLoopSourceRef source = CFRunLoopSourceCreate( nullptr, 0, &context );
		CFRunLoopAddSource( CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes );
		
		CFRunLoopRun();
		
		CFRunLoopRemoveSource( CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes );
		CFRelease( source );
	}
}

- (void) doBlock: (WorkBlock) block
{
	block();
}


@end

//MARK: -

void	PerformBlockOnWorkThread( WorkBlock block )
{
	Performer* performer = MakePerformer();
	static NSThread* sThread = MakeThread( performer );
	
	[performer performSelector: @selector(doBlock:)
				onThread: sThread
				withObject: block
				waitUntilDone: NO];
}


void	PerformBlockOnMainThread( WorkBlock block )
{
	Performer* performer = MakePerformer();
	
	[performer performSelectorOnMainThread: @selector(doBlock:)
				withObject: block
				waitUntilDone: NO];
}

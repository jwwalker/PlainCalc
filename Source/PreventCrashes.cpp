/*
 *  PreventCrashes.cpp
 *  PlainCalc2
 *
 *  Created by James Walker on 1/3/10.
 *  Copyright 2010 James W. Walker. All rights reserved.
 *
 */

#import "PreventCrashes.h"

#import <signal.h>
#import <cstdlib>


static void sighandler( int theSig )
{
	exit( theSig );
}


/*!
	@function	PreventCrashes
	
	@abstract	When the tool encounters various exceptions, just exit without
				triggering the creation of a crash report or dialog.
*/
void PreventCrashes()
{
	// Set up alternate stack for stack overflow case
	stack_t sigstk;
	sigstk.ss_sp = malloc(SIGSTKSZ);
	sigstk.ss_size = SIGSTKSZ;
	sigstk.ss_flags = 0;
	sigaltstack( &sigstk, NULL );
	
	// Types of exceptions we will intercept
	int	excepCodes[] =
	{
		SIGILL,
		SIGTRAP,
		SIGABRT,
		SIGFPE,
		SIGBUS,
		SIGSEGV
	};
	
	// Set up signal handlers
	struct sigaction act;
	act.sa_handler = sighandler;
	sigemptyset( &act.sa_mask );
	act.sa_flags = SA_ONSTACK;
	
	for (int i = 0; i < sizeof(excepCodes)/sizeof(excepCodes[0]); ++i)
	{
		sigaction( excepCodes[i], &act, NULL );
	}
}


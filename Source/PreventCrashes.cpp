/*
 *  PreventCrashes.cpp
 *  PlainCalc2
 *
 *  Created by James Walker on 1/3/10.
 *
 */
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

#import "PreventCrashes.h"

#import <signal.h>
#import <cstdlib>
#import <unistd.h>
#import <setjmp.h>
#import <cstdio>
#import <mach/exception_types.h>
#import <mach/mach_init.h>
#import <mach/task.h>


static sigjmp_buf                   jmpbuf;
static volatile sig_atomic_t        canjump;

static void sighandler( int theSig )
{
	if (canjump == 0)
		return;
	
	canjump = 0;
	siglongjmp( jmpbuf, theSig );
}


/*!
	@function	PreventCrashes
	
	@abstract	When the tool encounters various exceptions, just exit without
				triggering the creation of a crash report or dialog.
*/
void PreventCrashes()
{
	canjump = 0;
	
	task_set_exception_ports(
		mach_task_self(),
		EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC,
		MACH_PORT_NULL,
		EXCEPTION_STATE_IDENTITY,
		MACHINE_THREAD_STATE );
	
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
	
	// Jump back here if there is an exception
	int exCode = sigsetjmp( jmpbuf, 1 );
	if (exCode != 0)
	{
		fprintf( stderr, "Exiting CalcTool with code %d\n", exCode );
		exit( exCode );
	}
	canjump = 1;
}


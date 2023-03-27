#pragma once

#if __MACH__
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <CFDictionary.h>
#endif

/*!
	@header		ParseCalcLine.h
	
	This is a simple interface for a parsing calculator.
	You create a calculator state object with CreateCalcState,
	perform calculations by calling ParseCalcLine any number of
	times, and when finished call DisposeCalcState.
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


/*!
	@typedef	CalcState
	
	@abstract	Opaque pointer holding the state of a calculator.
*/
typedef struct SCalcState*	CalcState;

/*!
	@enum		ECalcResult
	
	@abstract	What was accomplished by a call to ParseCalcLine.
*/
enum ECalcResult
{
	kCalcResult_Error = 0,
	kCalcResult_Calculated,
	kCalcResult_DefinedFunction	
};

#ifdef __cplusplus
extern "C" {
#endif


/*!
	@function	ParseCalcLine
	@abstract	Attempt to parse and compute an expression or assignment.
	@param		inLine		A NUL-terminated line of text.
	@param		ioState		A calculator object reference.
	@param		outValue	Receives the computed value if the computation
							succeeded.
	@param		outStop		Offset at which parsing stopped, which can be
							helpful in spotting a syntax error.
	@param		outSymbol	If a function was defined, this receives the name of
							the function.  If a variable was assigned, this
							receives the name of the variable.  Otherwise, this
							receives the empty string.
	@result		Whether we failed, calculated, or defined a function.
*/
ECalcResult		ParseCalcLine( const char* inLine, CalcState ioState,
				double* outValue,
				long* outStop,
				const char** outSymbol = NULL );

/*!
	@function	CheckExpressionSyntax
	@abstract	Check the syntax of an expression.
	@param		inLine		A NUL-terminated line of text.
	@param		inState		A calculator object reference.
	@param		outStop		Offset at which parsing stopped, which can be
							helpful in spotting a syntax error.
	@result		True if the expression was parsed successfully.
*/
bool			CheckExpressionSyntax( const char* inLine, CalcState inState,
					long* outStop );


#ifdef __cplusplus
}
#endif

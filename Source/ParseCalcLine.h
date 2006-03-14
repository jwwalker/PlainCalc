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
	@function	CreateCalcState
	@abstract	Create and initialize a calculator.
	@result		A calculator object reference.
*/
CalcState	CreateCalcState();

/*!
	@function	DisposeCalcState
	@abstract	Dispose of a calculator object.
	@param		ioState		A calculator object reference.
*/
void		DisposeCalcState( CalcState ioState );

/*!
	@function	ParseCalcLine
	@abstract	Attempt to parse and compute an expression or assignment.
	@param		inLine		A NUL-terminated line of text.
	@param		ioState		A calculator object reference.
	@param		outValue	Receives the computed value if the computation
							succeeded.
	@param		outStop		Offset at which parsing stopped, which can be
							helpful in spotting a syntax error.
	@result		Whether we failed, calculated, or defined a function.
*/
ECalcResult		ParseCalcLine( const char* inLine, CalcState ioState,
				double* outValue,
				long* outStop );

/*!
	@function	CopyCalcVariables
	@abstract	Create a dictionary containing the variables (as CFStringRef keys)
				and their values (as CFNumberRef values).
	@param		inState		A calculator object reference.
	@result		A dictionary reference, or NULL on failure.
*/
CFDictionaryRef	CopyCalcVariables( CalcState inState );

/*!
	@function	SetCalcVariables
	@abstract	Add values of variable to a calculator.
	@param		inDict		A dictionary containing variable names (as
							CFStringRef keys) and their values (as CFNumberRef
							values).
	@param		ioState		A calculator object reference.
*/
void			SetCalcVariables( CFDictionaryRef inDict, CalcState ioState );

#ifdef __cplusplus
}
#endif

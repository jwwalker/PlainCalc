//  CalcStateFunctions.h
//  PlainCalc2
//
//  Created by James Walker on 3/27/23.
//  
//

#ifndef CalcStateFunctions_hpp
#define CalcStateFunctions_hpp

#include <CoreFoundation/CoreFoundation.h>

/*!
	@typedef	CalcState
	
	@abstract	Opaque pointer holding the state of a calculator.
*/
typedef struct SCalcState*	CalcState;

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

/*!
	@function	SetCalcVariable
	@abstract	Add or change a variable value in a calculator.
	@param		inVarName	Name of a variable (UTF-8).
	@param		inValue		Value to be assigned to the variable.
	@param		ioState		A calculator object reference.
*/
void			SetCalcVariable( const char* inVarName, double inValue,
								CalcState ioState );

/*!
	@function	CopyCalcFunctions
	@abstract	Create a dictionary recording user-defined functions.
	@discussion	The keys of the dictionary are the function names.  Each value
				is a CFArrray of CFStrings, being the function definition
				followed by the formal parameters.
	@param		inState		A calculator object reference.
	@result		A dictionary reference, or NULL on failure.
*/
CFDictionaryRef	CopyCalcFunctions( CalcState inState );

/*!
	@function	SetCalcFunctions
	@abstract	Add defined functions to the calculator.
	@discussion	The keys of the dictionary are the function names.  Each value
				is a CFArrray of CFStrings, being the function definition
				followed by the formal parameters.
	@param		inDict		A dictionary recording functions.
	@param		ioState		A calculator object reference.
*/
void			SetCalcFunctions( CFDictionaryRef inDict, CalcState ioState );


#ifdef __cplusplus
}
#endif


#endif /* CalcStateFunctions_hpp */

//  CalcStateFunctions.cpp
//  PlainCalc2
//
//  Created by James Walker on 3/27/23.
//  
//

#include "CalcStateFunctions.h"

#include "SCalcState.hpp"

CalcState	CreateCalcState()
{
	CalcState	theState = NULL;
	
	try
	{
		theState = new SCalcState;
	}
	catch (...)
	{
	}
	
	return theState;
}

void		DisposeCalcState( CalcState ioState )
{
	delete ioState;
}

/*!
	@function	CopyCalcVariables
	@abstract	Create a dictionary containing the variables (as CFStringRef keys)
				and their values (as CFNumberRef values).
	@param		inState		A calculator object reference.
	@result		A dictionary reference, or NULL on failure.
*/
CFDictionaryRef	CopyCalcVariables( CalcState inState )
{
	CFDictionaryRef	theDict = NULL;
	
	try
	{
		theDict = inState->CopyVariables();
	}
	catch (...)
	{
	}
	return theDict;
}

/*!
	@function	SetCalcVariables
	@abstract	Add values of variable to a calculator.
	@param		inDict		A dictionary containing variable names (as
							CFStringRef keys) and their values (as CFNumberRef
							values).
	@param		ioState		A calculator object reference.
*/
void			SetCalcVariables( CFDictionaryRef inDict, CalcState ioState )
{
	try
	{
		ioState->SetVariables( inDict );
	}
	catch (...)
	{
	}
}

/*!
	@function	SetCalcVariable
	@abstract	Add or change a variable value in a calculator.
	@param		inVarName	Name of a variable (UTF-8).
	@param		inValue		Value to be assigned to the variable.
	@param		ioState		A calculator object reference.
*/
void			SetCalcVariable( const char* inVarName, double inValue,
								CalcState ioState )
{
	try
	{
		ioState->mValStack.push_back( inValue );
		ioState->SetVariable( inVarName );
		ioState->mValStack.pop_back();
	}
	catch (...)
	{
	}
}

/*!
	@function	CopyCalcFunctions
	@abstract	Create a dictionary recording user-defined functions.
	@discussion	The keys of the dictionary are the function names.  Each value
				is a CFArrray of CFStrings, being the function definition
				followed by the formal parameters.
	@param		inState		A calculator object reference.
	@result		A dictionary reference, or NULL on failure.
*/
CFDictionaryRef	CopyCalcFunctions( CalcState inState )
{
	CFDictionaryRef	theDict = NULL;
	
	try
	{
		theDict = inState->CopyFuncDefs();
	}
	catch (...)
	{
	}
	return theDict;
}

/*!
	@function	SetCalcFunctions
	@abstract	Add defined functions to the calculator.
	@discussion	The keys of the dictionary are the function names.  Each value
				is a CFArrray of CFStrings, being the function definition
				followed by the formal parameters.
	@param		inDict		A dictionary recording functions.
	@param		ioState		A calculator object reference.
*/
void			SetCalcFunctions( CFDictionaryRef inDict, CalcState ioState )
{
	try
	{
		ioState->SetFuncDefs( inDict );
	}
	catch (...)
	{
	}
}

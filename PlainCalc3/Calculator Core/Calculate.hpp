//  Calculate.hpp
//  ParserPlay
//
//  Created by James Walker on 7/26/25.
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

#ifndef Calculate_hpp
#define Calculate_hpp

struct SCalcState;

#import <string>


enum class CalcInterruptCode : int
{
	none,			// not interrupted.
	userAbort,		// the user pressed the Escape key
	stackLimit		// Recursion exceeded a stack limit
};

enum class CalcResultType : int
{
	undefined,
	value,				// calculatedValue is set
	error,				// errorMessage is set
	interrupt,			// interruptCode is set
	definedFunc,		// funcName is set
	redefinedFunc		// funcName is set
};

/*!
	@struct		CalcResult
	
	@abstract	Result of a calculation.
	
	@discussion	The result of calculating an expression or an assignment to a variable
				is either a number (double) or a parsing error message or an interrupt code.  The
				result of calculating a function definition is the name of the function, in the successful
				case, or a parsing error message.
*/
struct CalcResult
{
	CalcResultType		type = CalcResultType::undefined;
	double				calculatedValue;
	std::string			errorMessage;
	CalcInterruptCode	interruptCode = CalcInterruptCode::none;
	std::string			funcName;
	
	void				SetValue( double inValue )
						{
							calculatedValue = inValue;
							type = CalcResultType::value;
						}
	void				SetInterrupt( CalcInterruptCode code )
						{
							interruptCode = code;
							type = CalcResultType::interrupt;
						}
	void				SetError( const std::string& message )
						{
							errorMessage = message;
							type = CalcResultType::error;
						}
	void				SetDefinedFunc( const std::string& name,
										bool isRedefined )
						{
							funcName = name;
							type = isRedefined?
								CalcResultType::redefinedFunc :
								CalcResultType::definedFunc;
						}
};



/*!
	@function	Calculate
	
	@abstract	Parse and execute a calculator statement.
	
	@param		inText		A line of input text.
	@param		ioState		A state object that may be used and modified in the course
							of the calculation.
	@result		The result of the calculation.
*/
CalcResult	Calculate( const std::string& inText, SCalcState& ioState );

#endif /* Calculate_hpp */

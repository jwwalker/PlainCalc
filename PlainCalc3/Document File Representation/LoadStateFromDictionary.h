//  LoadStateFromDictionary.h
//  PlainCalc3
//
//  Created by James Walker on 8/24/25.
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

#import <Foundation/Foundation.h>

class SCalcState;

/*!
	@function	LoadStateFromDictionary
	
	@abstract	Load the variables and user-defined functions from a dictionary representation.
	
	@discussion	Under the key @"variables" is a dictionary mapping variable names to numeric values.
				
				Under the key @"functions" is the PlainCalc2 representation of
				functions.  This is a dictionary mapping a function name to
				an array.  The first (zeroth) member of each array is the
				string representation of the right hand side of the function
				definition, and the remaining array members are the formal
				parameters of the function.
				
				Unfortunately, the version 2 representation of functions does
				not work well with the way user-defined functions are handled
				in version 3.  In order to restore a function, we need to
				calculate the function definition, and if some functions
				refer to others, the definitions must be added in a specific
				order.
				
				In version 3, we also have a key @"functions_v3" holding
				a dictionary mapping a function name to a dictionary
				representations of the syntax tree for the right hand side.
				Thus, to restore a function from a version 3 file, we use
				information from both @"functions_v3" and @"functions".
	
	@param		ioState		A state object, whose variables and userFunctions members will be
							modified.
	@param		inDict		A dictionary holding variables, functions, and other things that this
							function does not care about.
*/
void	LoadStateFromDictionary( SCalcState& ioState, NSDictionary* _Nonnull inDict );

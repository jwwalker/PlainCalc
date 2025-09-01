//  DoGetIndexVariable.hpp
//  PlainCalc3
//
//  Created by James Walker on 8/25/25.
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

#ifndef DoGetIndexVariable_h
#define DoGetIndexVariable_h

#import "Built-ins.hpp"
#import "SCalcState.hpp"
#import "MatchedText.hpp"

#import <algorithm>

struct DoGetIndexVariable
{
	void	operator()( auto& ctx ) const;
};

inline void	DoGetIndexVariable::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	std::string theIdentifier( MatchedText( ctx ) );
	bool failedToParse = false;
	std::string forbidMsg;
	
	// Do not allow the index variable to be a built-in identifier
	if ( BuiltInConstants().contains( theIdentifier ) or
		BuiltInUnarySyms().Contains( theIdentifier ) or
		BuiltInBinarySyms().Contains( theIdentifier ) or
		BuiltInNarySyms().Contains( theIdentifier ) or
		BuiltInIterationSyms().find( ctx, theIdentifier ) )
	{
		forbidMsg = "Built-in function or constant '" + theIdentifier +
			"' cannot be an index variable";
		failedToParse = true;
	}
	// If it is a user-defined variable or function, that is also asking
	// for trouble.
	else if (state.userFunctions.contains( theIdentifier ))
	{
		forbidMsg = "User-defined function '" + theIdentifier +
			"' cannot be an index variable";
		failedToParse = true;
	}
	else if (state.variables.contains( theIdentifier ))
	{
		forbidMsg = "User-defined variable '" + theIdentifier +
			"' cannot be an index variable";
		failedToParse = true;
	}
	// Do not allow an index variable to be a formal parameter of a function
	// being defined.
	else if (std::find( state.paramsOfFuncBeingDefined.cbegin(),
			state.paramsOfFuncBeingDefined.cend(), theIdentifier ) !=
			state.paramsOfFuncBeingDefined.cend())
	{
		forbidMsg = "Formal parameter '" + theIdentifier +
			"' cannot also be an index variable";
		failedToParse = true;
	}
	// Do not allow nested iterations to use the same index
	else if (std::find( state.iterationIndexVariables.cbegin(),
			state.iterationIndexVariables.cend(), theIdentifier ) !=
			state.iterationIndexVariables.cend())
	{
		forbidMsg = "Index variable '" + theIdentifier +
			"' cannot be used in two nested iterations";
		failedToParse = true;
	}

	if (failedToParse)
	{
		_report_error( ctx, forbidMsg );
		_pass(ctx) = false;
	}
	else
	{
		state.iterationIndexVariables.push_back( theIdentifier );
	}
}

#endif /* DoGetIndexVariable_h */

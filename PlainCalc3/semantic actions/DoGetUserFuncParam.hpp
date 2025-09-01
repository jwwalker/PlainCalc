//  DoGetUserFuncParam.hpp
//  ParserPlay
//
//  Created by James Walker on 8/8/25.
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

#ifndef DoGetUserFuncParam_h
#define DoGetUserFuncParam_h

#import "Built-ins.hpp"
#import "SCalcState.hpp"
#import "MatchedText.hpp"

#import <algorithm>

struct DoGetUserFuncParam
{
	void	operator()( auto& ctx ) const
	{
		SCalcState& state( _globals(ctx) );
		std::string theIdentifier( MatchedText( ctx ) );
		bool failedToParse = false;
		
		// Do not allow a formal parameter to match a built-in constant or
		// function or a user-defined function.
		std::string forbidMsg;
		if (BuiltInConstants().contains( theIdentifier ))
		{
			forbidMsg = "Built-in constant '" + theIdentifier +
				"' cannot be a formal parameter";
			failedToParse = true;
		}
		else if (state.userFunctions.contains( theIdentifier ))
		{
			forbidMsg = "User-defined function '" + theIdentifier +
				"' cannot be a formal parameter";
			failedToParse = true;
		}
		else if ( BuiltInUnarySyms().Contains( theIdentifier ) or
			BuiltInBinarySyms().Contains( theIdentifier ) or
			BuiltInNarySyms().Contains( theIdentifier ) or
			BuiltInIterationSyms().find( ctx, theIdentifier ) )
		{
			forbidMsg = "Built-in function '" + theIdentifier +
				"' cannot be a formal parameter";
			failedToParse = true;
		}
		
		// Do not allow two identical formal parameters.
		if (std::find( state.paramsOfFuncBeingDefined.cbegin(),
			state.paramsOfFuncBeingDefined.cend(), theIdentifier ) !=
			state.paramsOfFuncBeingDefined.cend())
		{
			forbidMsg = "Formal parameter '" + theIdentifier +
				"' cannot be used more than once,";
			failedToParse = true;
		}
		
		if (failedToParse)
		{
			_report_error( ctx, forbidMsg );
			_pass(ctx) = false;
		}
		else
		{
			state.paramsOfFuncBeingDefined.push_back( theIdentifier );
		}
	}
};

#endif /* DoGetUserFuncParam_h */

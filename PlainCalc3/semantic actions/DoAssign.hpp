//  DoAssign.hpp
//  ParserPlay
//
//  Created by James Walker on 7/23/25.
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

#ifndef DoAssign_h
#define DoAssign_h

#import "SCalcState.hpp"

struct DoAssign
{
	void	operator()( auto& ctx ) const;
};

inline void	DoAssign::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	// if a user-defined function has the name of leftIdentifier, erase it.
	state.userFunctions.erase( state.leftIdentifier );
	
	autoASTNode topNode( state.valStack.top() );
	std::optional<double> topValue = topNode->Evaluate( state );
	if (topValue.has_value())
	{
		state.variables[ state.leftIdentifier ] = topValue.value();
	}
	else
	{
		_pass( ctx ) = false;
		_report_error( ctx, "failed to get value to assign" );
	}
}

#endif /* DoAssign_h */

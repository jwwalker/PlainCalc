//  DoUserFuncDefine.hpp
//  ParserPlay
//
//  Created by James Walker on 7/27/25.
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

#ifndef DoUserFuncDefine_h
#define DoUserFuncDefine_h

#import "SCalcState.hpp"
#import "MatchedText.hpp"
#import "NumberNode.hpp"
#import <iostream>


/*!
	@struct		DoUserFuncDefine
	
	@abstract	Define a user-defined function.
	
	@discussion	If `false` is passed to the constructor, we are defining the function temporarily
				so that it can be recognized when parsing the right hand side of a recursive function
				definition.
*/
struct DoUserFuncDefine
{
	DoUserFuncDefine() = delete;
	
	DoUserFuncDefine( bool fullyDefined )
		: _fullyDefined( fullyDefined ) {}

	void	operator()( auto& ctx ) const
	{
		SCalcState& state( _globals(ctx) );
		
		// if a user-defined variable has the name of leftIdentifier,
		// erase it.
		state.variables.erase( state.leftIdentifier );
		
		std::string rhsText;
		if (_fullyDefined)
		{
			std::string wholeLine( MatchedText(ctx) );
			std::string::size_type equalOff = wholeLine.find('=');
			if (equalOff != std::string::npos)
			{
				equalOff += 1;
				while ( (equalOff < wholeLine.length()) and (wholeLine[equalOff] == ' '))
				{
					equalOff += 1;
				}
			}
			wholeLine.erase( 0, equalOff );
			rhsText = wholeLine;
		}
		
		// Record the function
		autoASTNode rightHandSide;
		if (_fullyDefined)
		{
			rightHandSide = state.valStack.top();
		}
		else
		{
			rightHandSide = autoASTNode( new NumberNode( 0.0 ) );
			state.preexistingUserFunc = state.userFunctions.contains( state.leftIdentifier );
		}
		state.userFunctions[ state.leftIdentifier ] =
			FuncDef( state.paramsOfFuncBeingDefined, rhsText, rightHandSide );
		state.definedUserFunc = _fullyDefined;
	}

private:
	bool	_fullyDefined;
};

#endif /* DoUserFuncDefine_h */

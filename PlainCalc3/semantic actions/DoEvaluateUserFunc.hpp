//  DoEvaluateUserFunc.hpp
//  ParserPlay
//
//  Created by James Walker on 8/3/25.
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

#ifndef DoEvaluateUserFunc_h
#define DoEvaluateUserFunc_h


#import "SCalcState.hpp"

#import "UserFuncNode.hpp"
#import "NumberNode.hpp"

#import <ranges> // for std::views::reverse
#import <sstream>
#import <algorithm>

struct DoEvaluateUserFunc
{
	void	operator()( auto& ctx ) const;
};

inline void	DoEvaluateUserFunc::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	// Get the name of the function
	if (state.funcNameStack.size() < 1)
	{
		_report_error( ctx, "function name stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	std::string funcName( state.funcNameStack.top() );
	state.funcNameStack.pop();
	
	// Look up the function definition
	const FuncDef& def( state.userFunctions[ funcName ] );
	const StringVec& formalParams( std::get<StringVec>(def) );
	autoASTNode rightHandSide( std::get<autoASTNode>(def) );
	
	// The most recent nodes on the stack should be the arguments of the
	// function, so if the number of such nodes is less than the number of
	// formal parameters, we are in trouble.
	if (state.valStack.size() < formalParams.size())
	{
		_report_error( ctx, "value stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	
	// Collect an argument for each formal parameter
	std::vector< autoASTNode > arguments;
	const size_t kParamCount = formalParams.size();
	for (size_t i = 0; i < kParamCount; ++i)
	{
		arguments.push_back( state.valStack.top() );
		state.valStack.pop();
	}
	
	// Put the arguments in forward order
	std::reverse( arguments.begin(), arguments.end() );
	
	autoASTNode userFuncNode( new UserFuncNode( funcName, arguments ) );
	
	if (state.suppressUserFuncEvaluation == 0)
	{
		std::optional<double> result = userFuncNode->Evaluate( state );
		
		if (result.has_value())
		{
			state.valStack.push( autoASTNode( new NumberNode( result.value() ) ) );
		}
		else
		{
			state.valStack.push( std::move( userFuncNode ) );
		}
	}
	else
	{
		state.valStack.push( std::move( userFuncNode ) );
	}
}

#endif /* DoEvaluateUserFunc_h */

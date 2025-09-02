//  DoEvaluateIteration.hpp
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

#ifndef DoEvaluateIteration_h
#define DoEvaluateIteration_h

#import "Built-ins.hpp"
#import "IterationNode.hpp"
#import "SCalcState.hpp"

struct DoEvaluateIteration
{
	void	operator()( auto& ctx ) const;
};

inline void	DoEvaluateIteration::operator()( auto& ctx ) const
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
	
	// Get the name of the index variable
	if (state.iterationIndexVariables.empty())
	{
		_report_error( ctx, "index variable stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	std::string indexVariable( state.iterationIndexVariables.back() );
	state.iterationIndexVariables.pop_back();
	
	auto kindVal = BuiltInIterationSyms().find( ctx, funcName );
	if (not kindVal)
	{
		_report_error( ctx, "unknown iteration kind" );
		_pass( ctx ) = false;
		return;
	}
	
	// Get the 3 arguments
	if (state.valStack.size() < 3)
	{
		_report_error( ctx, "value stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	autoASTNode contentNode( state.valStack.top() );
	state.valStack.pop();
	
	autoASTNode endValueNode( state.valStack.top() );
	state.valStack.pop();
	
	autoASTNode startValueNode( state.valStack.top() );
	state.valStack.pop();
	
	// Make an iteration node
	state.valStack.push( autoASTNode( new IterationNode( *kindVal,
		indexVariable, startValueNode, endValueNode, contentNode ) ) );
}

#endif /* DoEvaluateIteration_h */

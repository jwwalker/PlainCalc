//  DoEvaluateNary.hpp
//  ParserPlay
//
//  Created by James Walker on 8/10/25.
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

#ifndef DoEvaluateNary_h
#define DoEvaluateNary_h

#import "Built-ins.hpp"
#import "SCalcState.hpp"
#import "NumberNode.hpp"
#import "NaryFuncNode.hpp"
#import <math.h>

struct DoEvaluateNary
{
	void	operator()( auto& ctx ) const;
};

inline void	DoEvaluateNary::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	size_t attCount = _attr(ctx).end() - _attr(ctx).begin();
	
	if ( (state.valStack.size() < attCount) or
		(state.funcNameStack.size() < 1) )
	{
		_report_error( ctx, "stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	
	std::string funcName( state.funcNameStack.top() );
	state.funcNameStack.pop();
	
	NaryFunc theFunc = BuiltInNarySyms().at( funcName );
	
	// Pop the function arguments and put them into a vector.
	// This operation will reverse the order of the arguments, but none of
	// my n-ary functions care about the order of their arguments.
	std::vector< autoASTNode > args;
	args.reserve( attCount );
	while (attCount > 0)
	{
		autoASTNode oneArg( state.valStack.top() );
		state.valStack.pop();
		args.push_back( oneArg );
		--attCount;
	}
	
	autoASTNode funcNode( new NaryFuncNode( theFunc, args ) );
	
	std::optional<double> theValue = funcNode->Evaluate( state );
	if (theValue.has_value())
	{
		state.valStack.push( autoASTNode( new NumberNode( theValue.value() ) ) );
	}
	else
	{
		state.valStack.push( std::move( funcNode ) );
	}
}

#endif /* DoEvaluateNary_h */

//  DoEvaluateUnary.hpp
//  ParserPlay
//
//  Created by James Walker on 7/22/25.
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

#ifndef DoEvaluateUnary_h
#define DoEvaluateUnary_h

#import "Built-ins.hpp"
#import "SCalcState.hpp"
#import "NumberNode.hpp"
#import "UnaryFuncNode.hpp"

struct DoEvaluateUnary
{
	void	operator()( auto& ctx ) const;
};

inline void	DoEvaluateUnary::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	if ( (state.valStack.size() < 1) or (state.funcNameStack.size() < 1) )
	{
		_report_error( ctx, "stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	
	std::string funcName( state.funcNameStack.top() );
	state.funcNameStack.pop();
	
	UnaryFunc theFunc = BuiltInUnarySyms().at( funcName );

	autoASTNode paramNode = state.valStack.top();
	state.valStack.pop();
	
	std::optional<double> paramValue = paramNode->Evaluate( state );
	
	if (paramValue.has_value())
	{
		double result = theFunc( paramValue.value() );
		state.valStack.push( autoASTNode( new NumberNode( result ) ) );
	}
	else
	{
		state.valStack.push( autoASTNode( new UnaryFuncNode( theFunc, paramNode ) ) );
	}
}

#endif /* DoEvaluateUnary_h */

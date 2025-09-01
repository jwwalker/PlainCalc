//  DoEvaluateBinary.hpp
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

#ifndef DoEvaluateBinary_h
#define DoEvaluateBinary_h

#import "Built-ins.hpp"
#import "SCalcState.hpp"
#import "NumberNode.hpp"
#import "BinaryFuncNode.hpp"

struct DoEvaluateBinary
{
	void	operator()( auto& ctx ) const;
};

inline void	DoEvaluateBinary::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	if ( (state.valStack.size() < 2) or (state.funcNameStack.size() < 1) )
	{
		_report_error( ctx, "stack underrun" );
		_pass( ctx ) = false;
		return;
	}

	std::string funcName( state.funcNameStack.top() );
	state.funcNameStack.pop();
	BinaryFunc theFunc = BuiltInBinarySyms().at( funcName );

	autoASTNode param2Node( state.valStack.top() );
	state.valStack.pop();
	
	autoASTNode param1Node( state.valStack.top() );
	state.valStack.pop();
	
	std::optional<double> param1Value = param1Node->Evaluate( state );
	std::optional<double> param2Value = param2Node->Evaluate( state );
	
	if ( param1Value.has_value() and param2Value.has_value() )
	{
		double resultNum = theFunc( param1Value.value(), param2Value.value() );
		state.valStack.push( autoASTNode( new NumberNode( resultNum ) ) );
	}
	else
	{
		state.valStack.push( autoASTNode( new BinaryFuncNode( theFunc,
			param1Node, param2Node ) ) );
	}
}

#endif /* DoEvaluateBinary_h */

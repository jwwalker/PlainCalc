//  DoIf.hpp
//  ParserPlay
//
//  Created by James Walker on 8/7/25.
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

#ifndef DoIf_h
#define DoIf_h

#import "SCalcState.hpp"
#import "NumberNode.hpp"
#import "IfNode.hpp"

struct DoIfAtFirstComma
{
	void	operator()( auto& ctx ) const;
};


struct DoIfFinish
{
	void	operator()( auto& ctx ) const;
};

inline void	DoIfAtFirstComma::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	if (state.valStack.empty())
	{
		_report_error( ctx, "value stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	
	state.suppressUserFuncEvaluation += 1;
}

inline void	DoIfFinish::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	if (state.valStack.size() < 3)
	{
		_report_error( ctx, "stack underrun" );
		_pass( ctx ) = false;
		return;
	}
	
	autoASTNode noBranch( state.valStack.top() );
	state.valStack.pop();
	autoASTNode yesBranch( state.valStack.top() );
	state.valStack.pop();
	autoASTNode testBranch( state.valStack.top() );
	state.valStack.pop();
	
	std::optional<double> testVal( testBranch->Evaluate( state ) );
	
	if (testVal.has_value())
	{
		autoASTNode selectedNode = (testVal.value() > 0.0)? yesBranch : noBranch;
		state.valStack.push( std::move( selectedNode ) );
	}
	else
	{
		state.valStack.push( autoASTNode( new IfNode( testBranch, yesBranch,
			noBranch ) ) );
	}
	
	state.suppressUserFuncEvaluation -= 1;
}

#endif /* DoIf_h */

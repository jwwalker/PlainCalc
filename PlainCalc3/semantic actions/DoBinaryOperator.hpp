//  DoBinaryOperator.hpp
//  ParserPlay
//
//  Created by James Walker on 6/28/25.
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

#ifndef DoBinOp_h
#define DoBinOp_h

#import "SCalcState.hpp"

#import "NumberNode.hpp"
#import "BinaryFuncNode.hpp"

/*!
	@class		DoBinaryOperator
	@abstract	Semantic action that computes a binary operation.
	@discussion	The constructor parameter func should be a function of 2 double variables that
				returns a double.
*/
struct DoBinaryOperator
{
	DoBinaryOperator( BinaryFunc func ) : _func( func ) {}

	void	operator()( auto& ctx ) const
			{
				SCalcState& state( _globals(ctx) );
				if (state.valStack.size() < 2)
				{
					_report_error( ctx, "stack underrun" );
					_pass( ctx ) = false;
					return;
				}
				
				autoASTNode rhs( state.valStack.top() );
				state.valStack.pop();
				autoASTNode lhs( state.valStack.top() );
				state.valStack.pop();
				
				std::optional<double> leftVal( lhs->Evaluate( state ) );
				std::optional<double> rightVal( rhs->Evaluate( state ) );
				
				if ( leftVal.has_value() and rightVal.has_value() )
				{
					double resultNum = _func( leftVal.value(), rightVal.value() );
					state.valStack.push( autoASTNode( new NumberNode( resultNum ) ) );
				}
				else
				{
					state.valStack.push( autoASTNode( new BinaryFuncNode( _func, lhs, rhs ) ) );
				}
			}
	
private:
	BinaryFunc	_func;
};


#endif /* DoBinOp_h */

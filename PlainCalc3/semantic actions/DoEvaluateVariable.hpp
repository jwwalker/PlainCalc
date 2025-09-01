//  DoEvaluateVariable.hpp
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

#ifndef DoEvaluateConstant_h
#define DoEvaluateConstant_h

#import "Built-ins.hpp"
#import "IndexVariableNode.hpp"
#import "Lookup.hpp"
#import "SCalcState.hpp"
#import "MatchedText.hpp"
#import "NumberNode.hpp"
#import "ParameterIndexNode.hpp"

#import <iostream>
#import <algorithm>

struct DoEvaluateVariable
{
	void	operator()( auto& ctx ) const;
};


inline void	DoEvaluateVariable::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	std::string theIdentifier( MatchedText( ctx ) );
	
	// Is it a built-in constant?
	std::optional<double> constVal( Lookup( theIdentifier, BuiltInConstants() ) );
	if (constVal.has_value())
	{
		state.valStack.push( autoASTNode( new NumberNode( constVal.value() ) ) );
		return;
	}
	
	// If we are working on the right hand side of a user function definition,
	// we treat formal parameters as variables.  In any other situation,
	// paramsOfFuncBeingDefined will be empty.
	const auto vecIt = std::find( state.paramsOfFuncBeingDefined.cbegin(),
		state.paramsOfFuncBeingDefined.cend(), theIdentifier );

	if (vecIt != state.paramsOfFuncBeingDefined.cend())
	{
		unsigned int index = static_cast<unsigned int>(
			vecIt - state.paramsOfFuncBeingDefined.cbegin() );
		state.valStack.push( autoASTNode( new ParameterIndexNode( index ) ) );
		return;
	}
	
	// Is it a user-defined variable? (If there is a user-defined variable
	// with the same name as a formal parameter, the format parameter-ness
	// takes precedence.)
	std::optional<double> varValue( Lookup( theIdentifier, state.variables ) );
	if (varValue.has_value())
	{
		state.valStack.push( autoASTNode( new NumberNode( varValue.value() ) ) );
		return;
	}
	
	// Is it an iteration index variable?
	if (std::find( state.iterationIndexVariables.cbegin(),
		state.iterationIndexVariables.cend(), theIdentifier ) !=
		state.iterationIndexVariables.cend())
	{
		state.valStack.push( autoASTNode( new IndexVariableNode( theIdentifier ) ) );
		return;
	}
	
	// Is it "if"?  If we got here, we must not have reached any expectation
	// points in the "if" section of the factor parser, so it must not be
	// immediately followed by a left parenthesis.
	if (theIdentifier == "if")
	{
		_report_error( ctx, "'if' cannot be used as a variable, only as a "
			"built-in function" );
		_pass(ctx) = false;
		return;
	}
	
	// If the identifier is the name of a user-defined function, then we must
	// have already failed to parse it as a function, perhaps because the
	// wrong number of parameters were provided.  In that case, we want this
	// parse to fail, but there is no need for another error report.
	if (state.userFunctions.contains( theIdentifier ))
	{
		_pass(ctx) = false;
		return;
	}
	
	// If we have not returned by now, we do not know what this identifier is.
	std::string msg = "Identifier " + theIdentifier +
		" is not a known constant or variable";
	_report_error( ctx, msg );
	_pass(ctx) = false;
}

#endif /* DoEvaluateConstant_h */

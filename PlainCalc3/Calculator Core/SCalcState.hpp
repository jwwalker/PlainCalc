//  SCalcState.hpp
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

#ifndef SCalcState_hpp
#define SCalcState_hpp

#import "ASTNode.hpp"
#import "Built-ins.hpp"
#import "Calculate.hpp"

#import <stack>
#import <map>
#import <string>
#import <vector>
#import <utility>
#import <atomic>


using StringVec = std::vector< std::string >;

// This is everything but the function name in a function definition:
// A list of formal parameters, the right hand side as a string, and the
// right hand side as a syntax tree.
using FuncDef =				std::tuple< StringVec, std::string, autoASTNode >;

using ScalarMap =			std::map< std::string, double >;
using UnaryFunctionMap =	std::map< std::string, UnaryFunc >;
using BinaryFunctionMap =	std::map< std::string, BinaryFunc >;
using NaryFunctionMap =		std::map< std::string, NaryFunc >;
using UserFunctionMap =		std::map< std::string, FuncDef >;

enum class CalcType : int
{
	unknown,
	expression,
	variableAssignment,
	functionDefinition
};

struct SCalcState
{
							SCalcState();
							SCalcState( const SCalcState& other ) = delete;

	void					ClearTemporaries();
	
	// This is the data that needs to persist from one calculation to the next.
	ScalarMap					variables;
	UserFunctionMap				userFunctions;
	
	// The remaining members are used temporarily during parsing or
	// evaluation, and are reset by the ClearTemporaries method at the start
	// of a new calculation.
	std::stack< autoASTNode >	valStack;
	std::stack< std::string >	funcNameStack;
	std::string					leftIdentifier;
	
	StringVec					iterationIndexVariables;
	ScalarMap					indexVariableValues;
	StringVec					paramsOfFuncBeingDefined;
	std::vector<double>			functionArguments;
	bool						definedUserFunc;
	bool						preexistingUserFunc;
	int							suppressUserFuncEvaluation;
	size_t						maxStack;
	std::atomic< CalcInterruptCode >	interruptCode;
};


#endif /* SCalcState_hpp */

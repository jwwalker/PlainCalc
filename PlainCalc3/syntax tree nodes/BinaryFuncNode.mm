//  BinaryFuncNode.cpp
//  ParserPlay
//
//  Created by James Walker on 8/9/25.
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

#include "BinaryFuncNode.hpp"

#import "BasicMath.hpp"
#import "Built-ins.hpp"
#import "SCalcState.hpp"

#import <Foundation/Foundation.h>
#import <math.h>

std::optional<double>	BinaryFuncNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	std::optional<double> param1Value( _children[0]->Evaluate( state ) );
	std::optional<double> param2Value( _children[1]->Evaluate( state ) );
	
	if ( param1Value.has_value() and param2Value.has_value() )
	{
		result = _func( param1Value.value(), param2Value.value() );
	}
	
	return result;
}


autoCFDictionaryRef	BinaryFuncNode::ToDictionary() const
{
	NSDictionary* result = nil;
	std::string funcName;
	static std::map<BinaryFunc, std::string> basicOps = {
		{ Plus, "Plus" },
		{ Minus, "Minus" },
		{ Multiply, "Multiply" },
		{ Divide, "Divide" },
		{ ::pow, "^" }
	};
	
	// is _func a built-in binary function?
	for (const auto& [name, def] : BuiltInBinarySyms().Pairs())
	{
		if (def == _func)
		{
			funcName = name;
			break;
		}
	}
	
	// Is _func a basic math operation?
	if (funcName.empty())
	{
		const auto foundIt = basicOps.find( _func );
		if (foundIt != basicOps.end())
		{
			funcName = foundIt->second;
		}
	}
	
	if (not funcName.empty())
	{
		NSDictionary* param1 = CF_NS(_children[0]->ToDictionary());
		NSDictionary* param2 = CF_NS(_children[1]->ToDictionary());
		if ( (param1 != nil) and (param2 != nil) )
		{
			result = @{
				@"kind": @"BinaryFunc",
				@"name": @(funcName.c_str()),
				@"param1": param1,
				@"param2": param2
			};
		}
	}
	
	return NS_CF( result );
}


bool	BinaryFuncNode::operator==( const ASTNode& other ) const
{
	const BinaryFuncNode* asMyType = dynamic_cast<const BinaryFuncNode*>( &other );
	bool isEqual = (asMyType != nullptr) and
		(asMyType->GetFunc() == GetFunc()) and
		(*asMyType->Children()[0] == *Children()[0]) and
		(*asMyType->Children()[1] == *Children()[1]);
	return isEqual;
}

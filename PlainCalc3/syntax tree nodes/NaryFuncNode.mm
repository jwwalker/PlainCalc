//  NaryFuncNode.cpp
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

#import "NaryFuncNode.hpp"

#import "Built-ins.hpp"
#import "SCalcState.hpp"

#import <Foundation/Foundation.h>

std::optional<double>	NaryFuncNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	bool didEvaluateAll = true;
	
	std::vector<double> actualValues;
	actualValues.reserve( _children.size() );
	
	for (autoASTNode oneArg : _children)
	{
		std::optional<double> argVal = oneArg->Evaluate( state );
		if (argVal.has_value())
		{
			actualValues.push_back( argVal.value() );
		}
		else
		{
			didEvaluateAll = false;
			break;
		}
	}
	
	if (didEvaluateAll)
	{
		result = _func( actualValues );
	}
	
	return result;
}


autoCFDictionaryRef	NaryFuncNode::ToDictionary() const
{
	NSDictionary* result = nil;
	std::string funcName;
	
	for (const auto& [name, func] : BuiltInNarySyms().Pairs())
	{
		if (func == _func)
		{
			funcName = name;
			break;
		}
	}
	
	if (not funcName.empty())
	{
		NSMutableArray<NSDictionary*>* dicts = [NSMutableArray array];
		for (autoASTNode aNode : _children)
		{
			NSDictionary* argDict = CF_NS( aNode->ToDictionary() );
			if (argDict)
			{
				[dicts addObject: argDict];
			}
			else
			{
				dicts = nil;
				break;
			}
		}
		
		if (dicts != nil)
		{
			result = @{
				@"kind": @"NaryFunc",
				@"name": @(funcName.c_str()),
				@"args": dicts
			};
		}
	}
	
	return NS_CF( result );
}


bool	NaryFuncNode::operator==( const ASTNode& other ) const
{
	const NaryFuncNode* asMyType = dynamic_cast<const NaryFuncNode*>( &other );
	bool isEqual = (asMyType != nullptr) and
		(asMyType->GetFunc() == GetFunc()) and
		(asMyType->Children().size() == Children().size());
	
	if (isEqual)
	{
		const size_t  argCount = Children().size();
		
		for (size_t i = 0; i < argCount; ++i)
		{
			autoASTNode myArg( Children()[i] );
			autoASTNode otherArg( asMyType->Children()[i] );
			if (not (*otherArg == *myArg))
			{
				isEqual = false;
				break;
			}
		}
	}
	return isEqual;
}

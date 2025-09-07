//  UserFuncNode.cpp
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

#import "UserFuncNode.hpp"

#import "Lookup.hpp"
#import "SCalcState.hpp"
#import "GetStackSize.hpp"

#import <Foundation/Foundation.h>

#import <algorithm>

static constexpr size_t kStackLimit = 1048576U; // 1 megabyte

std::optional<double>	UserFuncNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	state.maxStack = std::max( state.maxStack, GetStackSize() );
	
	if (state.suppressUserFuncEvaluation > 0)
	{
		return result;
	}
	else if (state.interruptCode != CalcInterruptCode::none)
	{
		return result;
	}
	else if (state.maxStack > kStackLimit )
	{
		state.interruptCode = CalcInterruptCode::stackLimit;
		return result;
	}
	
	std::optional<FuncDef> userFunc( Lookup( _funcName, state.userFunctions ) );
	if (userFunc.has_value())
	{
		autoASTNode rhs = std::get<autoASTNode>( userFunc.value() );

		std::vector<double> arguments;
		arguments.reserve( _children.size() );
		
		for (autoASTNode argNode: _children)
		{
			std::optional<double> argVal( argNode->Evaluate( state ) );
			if (argVal.has_value())
			{
				arguments.push_back( argVal.value() );
			}
			else
			{
				break;
			}
		}
		
		if (arguments.size() == _children.size())
		{
			// See if we have previously cached the result of this evaluation.
			UserFuncCacheKey cacheKey( _funcName, arguments );
			auto foundIt = state.resultCache.find( cacheKey );
			if (foundIt == state.resultCache.end())
			{
				state.functionArguments.swap( arguments );
				
				result = rhs->Evaluate( state );

				state.functionArguments.swap( arguments );
				
				state.resultCache.emplace( std::move(cacheKey), *result  );
			}
			else // use cached result
			{
				result = foundIt->second;
			}
		
		}
	}
	
		
	return result;
}


autoCFDictionaryRef	UserFuncNode::ToDictionary() const
{
	NSDictionary* result = nil;
	
	NSMutableArray<NSDictionary*>* dicts = [NSMutableArray array];
	for (autoASTNode aNode : _children)
	{
		NSDictionary* argDict = CF_NS(aNode->ToDictionary());
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
			@"kind": @"UserFunc",
			@"name": @(_funcName.c_str()),
			@"args": dicts
		};
	}
	
	return NS_CF( result );
}


bool	UserFuncNode::operator==( const ASTNode& other ) const
{
	const UserFuncNode* asMyType = dynamic_cast<const UserFuncNode*>( &other );
	bool isEqual = (asMyType != nullptr) and
		(asMyType->_funcName == _funcName) and
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

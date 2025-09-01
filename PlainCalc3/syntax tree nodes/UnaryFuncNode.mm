//  UnaryFuncNode.cpp
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

#import "UnaryFuncNode.hpp"

#import "BasicMath.hpp"
#import "Built-ins.hpp"
#import "SCalcState.hpp"

#import <Foundation/Foundation.h>

std::optional<double>	UnaryFuncNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	std::optional<double> paramValue( _children[0]->Evaluate( state ) );
	
	if (paramValue.has_value())
	{
		result = _func( paramValue.value() );
	}
	
	return result;
}


autoCFDictionaryRef	UnaryFuncNode::ToDictionary() const
{
	NSDictionary* result = nil;
	std::string funcName;
	
	if (_func == Negate)
	{
		funcName = "Negate";
	}
	else
	{
		// is _func a built-in unary function?
		for (const auto& [name, def] : BuiltInUnarySyms().Pairs() )
		{
			if (def == _func)
			{
				funcName = name;
				break;
			}
		}
	}
	
	if (not funcName.empty())
	{
		NSDictionary* paramDict = CF_NS(_children[0]->ToDictionary());
		if (paramDict)
		{
			result = @{
				@"kind": @"UnaryFunc",
				@"name": @(funcName.c_str()),
				@"param": paramDict
			};
		}
	}
	
	return NS_CF( result );
}


bool	UnaryFuncNode::operator==( const ASTNode& other ) const
{
	const UnaryFuncNode* asMyType = dynamic_cast<const UnaryFuncNode*>( &other );
	bool isEqual = (asMyType != nullptr) and
		(asMyType->GetFunc() == GetFunc()) and
		(*asMyType->Children()[0] == *Children()[0]);
	return isEqual;
}

//  IfNode.cpp
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

#import "IfNode.hpp"

#import <Foundation/Foundation.h>

std::optional<double>	IfNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	std::optional<double> testVal( _children[0]->Evaluate( state ) );
	if (testVal.has_value())
	{
		if ( testVal.value() > 0.0 )
		{
			std::optional<double> val1( _children[1]->Evaluate( state ) );
			if (val1.has_value())
			{
				result = val1.value();
			}
		}
		else
		{
			std::optional<double> val2( _children[2]->Evaluate( state ) );
			if (val2.has_value())
			{
				result = val2.value();
			}
		}
	}
	
	return result;
}


autoCFDictionaryRef	IfNode::ToDictionary() const
{
	NSDictionary* result = nil;
	
	NSDictionary* testDict = CF_NS( _children[0]->ToDictionary() );
	NSDictionary* param1dict = CF_NS( _children[1]->ToDictionary() );
	NSDictionary* param2dict = CF_NS( _children[2]->ToDictionary() );
	
	if ( (testDict != nil) and (param1dict != nil) and (param2dict != nil) )
	{
			result = @{
				@"kind": @"If",
				@"test": testDict,
				@"param1": param1dict,
				@"param2": param2dict
			};
	}
	
	return NS_CF( result );
}


bool	IfNode::operator==( const ASTNode& other ) const
{
	const IfNode* asMyType = dynamic_cast<const IfNode*>( &other );
	bool isEqual = (asMyType != nullptr) and
		(*asMyType->Children()[0] == *Children()[0]) and
		(*asMyType->Children()[1] == *Children()[1]) and
		(*asMyType->Children()[2] == *Children()[2]);
	return isEqual;
}

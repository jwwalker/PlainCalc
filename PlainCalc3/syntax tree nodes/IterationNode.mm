//  IterationNode.mm
//  PlainCalc3
//
//  Created by James Walker on 8/25/25.
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

#import "IterationNode.hpp"

#import "SCalcState.hpp"

#import <Foundation/Foundation.h>

std::optional<double>	IterationNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	std::optional<double> startVal( _children[0]->Evaluate( state ) );
	std::optional<double> endVal( _children[1]->Evaluate( state ) );
	
	if (startVal.has_value() and endVal.has_value())
	{
		double startNum = startVal.value();
		double endNum = endVal.value();
		double total = (Kind() == IterationKind::summation)? 0.0 : 1.0;
		BOOL allEvaluated = YES;
		
		for (double i = startNum; i <= endNum; ++i)
		{
			if (state.interruptCode != CalcInterruptCode::none)
			{
				allEvaluated = NO;
				break;
			}
			state.indexVariableValues[ Variable() ] = i;
			std::optional<double> contentVal( _children[2]->Evaluate( state ) );
			if (contentVal.has_value())
			{
				double theContent = contentVal.value();
				if (Kind() == IterationKind::summation)
				{
					total += theContent;
				}
				else
				{
					total *= theContent;
				}
			}
			else
			{
				allEvaluated = NO;
				break;
			}
		}
		state.indexVariableValues.erase( Variable() );
		
		if (allEvaluated)
		{
			result = total;
		}
	}
	
	return result;
}

autoCFDictionaryRef		IterationNode::ToDictionary() const
{
	NSDictionary* result = nil;
	NSDictionary* start = CF_NS(_children[0]->ToDictionary());
	NSDictionary* end = CF_NS(_children[1]->ToDictionary());
	NSDictionary* content = CF_NS(_children[2]->ToDictionary());
	
	if ( (start != nil) and (end != nil) and (content != nil) )
	{
		result = @{
			@"kind": @"IterationNode",
			@"subtype" : @( static_cast<int>(Kind()) ),
			@"variable": @( Variable().c_str() ),
			@"start": start,
			@"end": end,
			@"content": content
		};
	}
	
	return NS_CF( result );
}

bool	IterationNode::operator==( const ASTNode& other ) const
{
	const IterationNode* asMyType = dynamic_cast<const IterationNode*>( &other );
	bool isEqual = (asMyType != nullptr) and
		(asMyType->Kind() == Kind()) and
		(asMyType->Variable() == Variable()) and
		(*asMyType->Children()[0] == *Children()[0]) and
		(*asMyType->Children()[1] == *Children()[1]) and
		(*asMyType->Children()[2] == *Children()[2]);
	return isEqual;
}

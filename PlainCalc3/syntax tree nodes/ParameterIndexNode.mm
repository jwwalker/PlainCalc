//  ParameterIndexNode.cpp
//  PlainCalc3
//
//  Created by James Walker on 8/23/25.
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

#import "ParameterIndexNode.hpp"

#import "SCalcState.hpp"

#import <Foundation/Foundation.h>

std::optional<double>	ParameterIndexNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result;
	
	if (_index < state.functionArguments.size())
	{
		result = state.functionArguments[ _index ];
	}
	
	return result;
}

autoCFDictionaryRef	ParameterIndexNode::ToDictionary() const
{
	NSDictionary* result = @{
		@"kind": @"ParameterIndex",
		@"index": @(_index)
	};
	return NS_CF( result );
}


bool	ParameterIndexNode::operator==( const ASTNode& other ) const
{
	const ParameterIndexNode* asMyType = dynamic_cast<const ParameterIndexNode*>( &other );
	return (asMyType != nullptr) and
		(asMyType->Index() == Index());
}

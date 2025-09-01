//  IndexVariableNode.mm
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

#import "IndexVariableNode.hpp"

#import "Lookup.hpp"
#import "SCalcState.hpp"

#import <Foundation/Foundation.h>

std::optional<double>	IndexVariableNode::Evaluate( SCalcState& state ) const
{
	std::optional<double> result = Lookup( _name, state.indexVariableValues );
	
	return result;
}

autoCFDictionaryRef		IndexVariableNode::ToDictionary() const
{
	NSDictionary* result = @{
		@"kind": @"IndexVariable",
		@"name": @(_name.c_str())
	};
	return NS_CF( result );
}

bool	IndexVariableNode::operator==( const ASTNode& other ) const
{
	const IndexVariableNode* asMyType = dynamic_cast<const IndexVariableNode*>( &other );
	return (asMyType != nullptr) and
		(asMyType->Name() == Name());
}

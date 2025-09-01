//  ASTNode.hpp
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

#ifndef ASTNode_hpp
#define ASTNode_hpp

#import <optional>
#import <memory>
#import <vector>
#import <initializer_list>
#import "autoCF.hpp"

struct SCalcState;

using autoASTNode = std::shared_ptr<class ASTNode>;

using ASTNodeVec = std::vector< autoASTNode >;

/// Abstract base class for a syntax tree node.
class ASTNode
{
public:
				ASTNode() = default;
				ASTNode( const ASTNodeVec& vec )
					: _children( vec ) {}
				ASTNode( std::initializer_list<autoASTNode> list )
					: _children( list ) {}
				ASTNode( const ASTNode& o ) = delete;
	
	virtual		~ASTNode() = default;
	
	unsigned int					Count() const noexcept;
	
	virtual std::optional<double>	Evaluate( SCalcState& state ) const = 0;
	
	virtual autoCFDictionaryRef		ToDictionary() const = 0;
	
	virtual bool					operator==( const ASTNode& other ) const = 0;
	
	const ASTNodeVec&				Children() const noexcept { return _children; }

protected:
	ASTNodeVec						_children;
};

inline  unsigned int	ASTNode::Count() const noexcept
{
	unsigned int result = 1;
	for (auto child: _children)
	{
		result += child->Count();
	}
	return result;
}

#endif /* ASTNode_hpp */

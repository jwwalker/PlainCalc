//  UnaryFuncNode.hpp
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

#ifndef UnaryFuncNode_hpp
#define UnaryFuncNode_hpp

#import "ASTNode.hpp"
#import "Built-ins.hpp"		// for UnaryFunc

class UnaryFuncNode : public ASTNode
{
public:
			UnaryFuncNode( double (*func)( double ), autoASTNode param )
				: ASTNode{ param }
				, _func( func ) {}
	
	std::optional<double>	Evaluate( SCalcState& state ) const override;
	
	autoCFDictionaryRef		ToDictionary() const override;
	
	bool					operator==( const ASTNode& other ) const override;
	
	UnaryFunc				GetFunc() const { return _func; }
	
private:
	UnaryFunc		_func;
};


#endif /* UnaryFuncNode_hpp */

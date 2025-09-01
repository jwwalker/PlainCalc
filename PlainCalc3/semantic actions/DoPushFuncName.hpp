//  DoPushFuncName.hpp
//  PlainCalc3
//
//  Created by James Walker on 8/30/25.
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

#ifndef DoPushFuncName_h
#define DoPushFuncName_h

#import "SCalcState.hpp"
#import "MatchedText.hpp"

struct DoPushFuncName
{
	void	operator()( auto& ctx ) const;
};

/// Check whether the identifier is a name of a built-in unary function, and if so, push it on the
/// function name stack.
inline void	DoPushFuncName::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	std::string theIdentifier( MatchedText( ctx ) );
	
	theIdentifier.resize( theIdentifier.size() - 1 );	// trim '('
	
	state.funcNameStack.push( theIdentifier );
}

#endif /* DoPushFuncName_h */

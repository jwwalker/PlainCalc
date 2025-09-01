//  DoCheckForUserFunc.hpp
//  ParserPlay
//
//  Created by James Walker on 8/3/25.
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

#ifndef DoCheckForUserFunc_h
#define DoCheckForUserFunc_h


#import "SCalcState.hpp"
#import "MatchedText.hpp"


/// Check whether the identifier is a name of a user function, and if so, push it on the
/// function name stack.
struct DoCheckForUserFunc
{
	void	operator()( auto& ctx ) const
	{
		SCalcState& state( _globals(ctx) );
		std::string theIdentifier( MatchedText( ctx ) );
		
		if (state.userFunctions.contains( theIdentifier ))
		{
			state.funcNameStack.push( theIdentifier );
		}
		else
		{
			_pass(ctx) = false;
		}
	}
};

#endif /* DoCheckForUserFunc_h */

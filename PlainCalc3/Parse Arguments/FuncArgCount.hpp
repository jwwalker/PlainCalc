//  FuncArgCount.hpp
//  ParserPlay
//
//  Created by James Walker on 8/11/25.
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

#ifndef FuncArgCount_h
#define FuncArgCount_h

struct FuncArgCount
{
	int operator()( auto& ctx ) const;
};


// Return the number of arguments, minus 1, for the user-defined function
// currently being parsed.
inline int FuncArgCount::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	
	std::string funcName( state.funcNameStack.top() );

	const FuncDef& funcInfo( state.userFunctions[ funcName ] );
	const StringVec& params( std::get<StringVec>( funcInfo ) );
	return static_cast<int>(params.size()) - 1;
}

#endif /* FuncArgCount_h */

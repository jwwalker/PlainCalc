//  DoBinaryFunc.cpp
//  PlainCalc2
//
//  Created by James Walker on 3/26/23.
//  
//
/*
	Copyright (c) 2006-2023 James W. Walker

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

#include "DoBinaryFunc.hpp"

#include "CalcException.hpp"
#include "SCalcState.hpp"
#include "SFixedSymbols.hpp"

#include <string>

void	DoBinaryFunc::operator()( const char* inStart, const char* inEnd ) const
{
	std::string	parsedText( inStart, inEnd );
	std::string::size_type	parenLoc = parsedText.find( '(' );
	if (parenLoc == std::string::npos)
	{
		throw CalcException();
	}
	parsedText.erase( parenLoc );
	const BinaryFunc*	foundFunc = find( mState.mFixed.mBinaryFuncs, parsedText.c_str() );
	if (foundFunc == NULL)
	{
		throw CalcException();
	}
	ThrowIfEmpty_( mState.mValStack );
	double	param2 = mState.mValStack.back();
	mState.mValStack.pop_back();
	ThrowIfEmpty_( mState.mValStack );
	double	param1 = mState.mValStack.back();
	mState.mValStack.pop_back();
	mState.mValStack.push_back( (*foundFunc)( param1, param2 ) );
}

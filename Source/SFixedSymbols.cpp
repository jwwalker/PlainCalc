//  SFixedSymbols.cpp
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

#include "SFixedSymbols.hpp"

#import <cmath>

static double rad( double inDegrees )
{
	return inDegrees * (M_PI / 180.0);
}

static double deg( double inRadians )
{
	return inRadians * (180.0 / M_PI);
}


SFixedSymbols::SFixedSymbols()
{
	mUnaryFuncs.add
		( "atan", std::atan )
		( "acos", std::acos )
		( "asin", std::asin )
		( "sin", std::sin )
		( "cos", std::cos )
		( "tan", std::tan )
		( "ceil", std::ceil )
		( "floor", std::floor )
		( "round", round )
		( "fabs", std::fabs )
		( "abs", std::fabs )
		( "log", std::log )
		( "log10", std::log10 )
		( "log2", log2 )
		( "exp", std::exp )
		( "rad", rad )
		( "deg", deg )
		( "sqrt", std::sqrt )
		( "\xE2\x88\x9A", std::sqrt )	// sqrt character in UTF-8
		;
	
	mBinaryFuncs.add
		( "atan2", std::atan2 )
		( "max", fmax )
		( "min", fmin )
		( "hypot", hypot )
		;
	
	const double	piVal = 4.0 * atan( 1.0 );
	
	mConstants.add
		( "pi", piVal )
		( "%", 0.01 )
		( "\xCF\x80", piVal )	// the pi character in UTF-8
		( "e", std::exp(1.0) )
		;
}


const SFixedSymbols&	GetFixedSyms()
{
	static const SFixedSymbols*	sSyms = NULL;
	if (sSyms == NULL)
	{
		sSyms = new SFixedSymbols;
	}
	return *sSyms;
}

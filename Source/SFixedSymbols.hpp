//  SFixedSymbols.hpp
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

#ifndef SFixedSymbols_hpp
#define SFixedSymbols_hpp

#pragma warn_unusedarg	off
#if !defined(BOOST_SPIRIT_USE_OLD_NAMESPACE)
#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#endif
#include "boost/spirit/include/classic.hpp"
#pragma warn_unusedarg	reset

typedef		double (*UnaryFunc)( double );
typedef		double (*BinaryFunc)( double, double );


/*!
	#struct		SFixedSymbols
	
	@abstract	Structure holding sybold tables for built-in constants and functions.
*/
struct SFixedSymbols
{
							SFixedSymbols();
							
	boost::spirit::symbols<UnaryFunc>	mUnaryFuncs;
	boost::spirit::symbols<BinaryFunc>	mBinaryFuncs;
	boost::spirit::symbols<double>		mConstants;
};

const SFixedSymbols&	GetFixedSyms( void );

#endif /* SFixedSymbols_hpp */
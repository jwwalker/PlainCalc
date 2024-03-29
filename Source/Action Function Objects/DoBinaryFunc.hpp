//  DoBinaryFunc.hpp
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

#ifndef DoBinaryFunc_hpp
#define DoBinaryFunc_hpp

struct SCalcState;

#ifndef ThrowIfEmpty_
#define	ThrowIfEmpty_( x ) do { if (x.empty()) throw CalcException(); } while (false)
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <boost/range.hpp>
#pragma clang diagnostic pop

/*!
	@struct		DoBinaryFunc
	@abstract	Functor for a semantic action that computes a binary function
				stored in a symbol table.
*/
struct DoBinaryFunc
{
			DoBinaryFunc( SCalcState& ioState ) : mState( ioState ) {}
			DoBinaryFunc( const DoBinaryFunc& inOther ) : mState( inOther.mState ) {}
	
	void	operator()( boost::iterator_range<const char*>& matchRange ) const;
	
	SCalcState&		mState;
};

#endif /* DoBinaryFunc_hpp */

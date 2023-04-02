//  DoAppendNumber.hpp
//  PlainCalc2
//
//  Created by James Walker on 3/28/23.
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

#ifndef DoAppendNumber_hpp
#define DoAppendNumber_hpp

struct SCalcState;

namespace boost
{
	namespace spirit
	{
		struct unused_type;
	}
}

using boost::spirit::unused_type;


/*!
	@struct		DoAppendNumber
	@abstract	Functor for a semantic action that assigns the value of
				an expression to a variable.
*/
struct DoAppendNumber
{
		DoAppendNumber( SCalcState& ioState ) : mState( ioState ) {}
		DoAppendNumber( const DoAppendNumber& inOther ) : mState( inOther.mState ) {}
	
	void	operator()( double val, unused_type, unused_type ) const;
	
	SCalcState&		mState;
};


#endif /* DoAppendNumber_hpp */

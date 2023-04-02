//  DoGetMatchedString.hpp
//  PlainCalc2
//
//  Created by James Walker on 3/30/23.
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

#ifndef DoGetMatchedString_hpp
#define DoGetMatchedString_hpp

#include <boost/range.hpp>
#include <string>

/*!
	@struct		DoGetMatchedString
	
	@abstract	Semantic action to copy the text matched by a parser enclosed in a raw[] directive
				to a string variable in the calculator state.
*/
struct DoGetMatchedString
{
		DoGetMatchedString( std::string& target )
			: _target( target ) {}
	
	inline void operator()( boost::iterator_range<const char*>& s ) const
	{
		_target.assign( s.begin(), s.end() );
	}
	
	std::string&	_target;
};

#endif /* DoGetMatchedString_hpp */

//  DoGetAssignableIdentifier.hpp
//  ParserPlay
//
//  Created by James Walker on 7/23/25.
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

#ifndef DoGetAssignableIdentifier_h
#define DoGetAssignableIdentifier_h

#import "Built-ins.hpp"
#import "SCalcState.hpp"
#import "MatchedText.hpp"


/*!
	@struct		DoGetAssignableIdentifier
	
	@abstract	If the matched identifier is not the name of a built-in variable or function,
				then pass it and record its name, otherwise fail.
*/
struct DoGetAssignableIdentifier
{
	void	operator()( auto& ctx ) const;
};

inline void	DoGetAssignableIdentifier::operator()( auto& ctx ) const
{
	SCalcState& state( _globals(ctx) );
	std::string theIdentifier( MatchedText( ctx ) );
	
	if ( BuiltInConstants().contains( theIdentifier ) or
		BuiltInUnarySyms().Contains( theIdentifier ) or
		BuiltInBinarySyms().Contains( theIdentifier ) or
		BuiltInNarySyms().Contains( theIdentifier ) or
		BuiltInIterationSyms().find( ctx, theIdentifier ) or
		(theIdentifier == "if") )
	{
		std::string msg = "Identifier '" + theIdentifier + "' is built in, " +
			"and cannot be assigned to,";
		_report_error( ctx, msg );
		_pass( ctx ) = false;
	}
	else
	{
		state.leftIdentifier = theIdentifier;
	}
}

#endif /* DoGetAssignableIdentifier_h */

//  DoDefinedFunc.cpp
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

#include "DoDefinedFunc.hpp"

#include "CalcException.hpp"
#include "ParseCalcLine.h"
#include "SCalcState.hpp"
#include "SFixedSymbols.hpp"

#include <string>

void	DoDefinedFunc::operator()( boost::iterator_range<const char*>& matchRange ) const
{
	// Find the name of the function
	std::string	parsedText( matchRange.begin(), matchRange.end() );
	std::string::size_type	parenLoc = parsedText.find( '(' );
	if (parenLoc == std::string::npos)
	{
		throw CalcException();
	}
	parsedText.erase( parenLoc );
	
	// Find the definition
	const FuncDef*	foundFunc = mState.mFuncDefs.find( parsedText.c_str() );
	if (foundFunc == NULL)
	{
		throw CalcException();
	}
	
	// Set values of the formal parameters in a temporary state
	const StringVec&	formalParams( foundFunc->first );
	if (mState.mValStack.size() < formalParams.size())
	{
		throw CalcException();
	}
	SCalcState	tempState( mState );
	for (StringVec::const_reverse_iterator i = formalParams.rbegin();
		i != formalParams.rend(); ++i)
	{
		const std::string&	theParam( *i );
		tempState.SetVariable( theParam.c_str() );
		tempState.mValStack.pop_back();
		mState.mValStack.pop_back();
	}
	
	// Evaluate
	double	funcVal;
	long	stopOff;
	ECalcResult	didParse = ParseCalcLine( foundFunc->second.c_str(),
		&tempState, &funcVal, &stopOff );
	if (didParse == kCalcResult_Calculated)
	{
		mState.mValStack.push_back( funcVal );
	}
	else
	{
		throw CalcException();
	}
}

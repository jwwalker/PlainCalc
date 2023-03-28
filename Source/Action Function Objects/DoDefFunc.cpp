//  DoDefFunc.cpp
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

#include "DoDefFunc.hpp"

#include "CalcException.hpp"
#include "ParseCalcLine.h"
#include "SCalcState.hpp"
#include "SFixedSymbols.hpp"

#include <string>

void	DoDefFunc::operator()( unused_type, unused_type, unused_type ) const
{
	// Before committing to the function, syntax check it.
	SCalcState	tempState( mState );
	tempState.SetFunc( mState.mFuncName, mState.mParamStack,
		mState.mFuncDef );

	// Find the definition
	FuncDef*	foundFunc = tempState.mFuncDefs.find( mState.mFuncName.c_str() );
	if (foundFunc == NULL)
	{
		throw CalcException();
	}

	// Set values of the formal parameters in the temporary
	// state.  It does not matter what I set them to, they
	// just need to be recognized as variables.
	const StringVec&	formalParams( foundFunc->first );
	tempState.mValStack.push_back( 1.0 );
	for (StringVec::const_iterator i = formalParams.begin();
		i != formalParams.end(); ++i)
	{
		const std::string&	theParam( *i );
		tempState.SetVariable( theParam.c_str() );
	}

	long	stopOff;
	if (CheckExpressionSyntax( foundFunc->second.c_str(),
		&tempState, &stopOff ))
	{
		mState.SetFunc( mState.mFuncName, mState.mParamStack,
			mState.mFuncDef );
		mState.mParamStack.clear();
		mState.mDidDefineFunction = true;
		mState.mDefinedSymbol = mState.mFuncName;
	}
	else
	{
		throw CalcException();
	}
}

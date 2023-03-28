//  SCalcState.hpp
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

#ifndef SCalcState_hpp
#define SCalcState_hpp

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wcomma"

#include <boost/spirit/include/qi.hpp>

#pragma clang diagnostic pop


#include <CoreFoundation/CoreFoundation.h>
#include <string>
#include <vector>
#include <set>
#include <utility>
struct SFixedSymbols;

typedef		std::vector<double>	 		DblStack;
typedef		std::vector< std::string >	StringVec;
typedef		std::set< std::string >		StringSet;

typedef		std::pair< StringVec, std::string >		FuncDef;


/*!
	@struct		SCalcState
	
	@abstract	Structure holding the current state of the calculator.
*/
struct SCalcState
{
							SCalcState();
							SCalcState( const SCalcState& inOther );
						
	void					SetVariable( const char* inVarName );
	void					SetFunc( const std::string& inFuncName,
									const StringVec& inFormalParams,
									const std::string& inValue );
	
	CFDictionaryRef			CopyVariables() const;
	void					SetVariables( CFDictionaryRef inDict );
	
	CFDictionaryRef			CopyFuncDefs() const;
	void					SetFuncDefs( CFDictionaryRef inDict );
	
	const SFixedSymbols&	mFixed;
	
	DblStack				mValStack;
	StringVec				mParamStack;
	boost::spirit::qi::symbols<char, FuncDef>	mFuncDefs;
	boost::spirit::qi::symbols<char, double>	mVariables;
	std::string				mIdentifier;
	StringSet				mVariableSet;
	StringSet				mFuncDefSet;
	std::string				mFuncName;
	std::string				mFuncParam;
	std::string				mFuncDef;
	std::string				mIf1;
	std::string				mIf2;
	std::string				mDefinedSymbol;
	bool					mDidDefineFunction;
};


#endif /* SCalcState_hpp */

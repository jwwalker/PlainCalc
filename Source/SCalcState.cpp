//  SCalcState.cpp
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

#include "SCalcState.hpp"

#include "autoCF.h"
#include "CFStringToUTF8.h"
#include "SFixedSymbols.hpp"
#include "UTF8ToCFString.h"

#include <string.h>

#define	ThrowIfCFFail_( x ) do { if ((x) == NULL) throw std::bad_alloc(); } while (false)

static constexpr char kParamPrefixChar = '#';

SCalcState::SCalcState()
	: mFixed( GetFixedSyms() )
	, mDidDefineFunction( false )
{
}

SCalcState::SCalcState( const SCalcState& inOther )
	: mFixed( inOther.mFixed )
	, mValStack( inOther.mValStack )
	, mParamStack( inOther.mParamStack )
	, mFuncDefs()
	, mVariables()
	, mIdentifier( inOther.mIdentifier )
	, mVariableSet( inOther.mVariableSet )
	, mFuncDefSet( inOther.mFuncDefSet )
	, mFuncName( inOther.mFuncName )
	, mFuncParam( inOther.mFuncParam )
	, mFuncDef( inOther.mFuncDef )
	, mIf1( inOther.mIf1 )
	, mIf2( inOther.mIf2 )
	, mDidDefineFunction( inOther.mDidDefineFunction )
{
	// Apparently, the copy constructor of symbols does not do a deep copy,
	// so let's try using default construction and copy assignment instead.
	mFuncDefs = inOther.mFuncDefs;
	mVariables = inOther.mVariables;
}

void	SCalcState::SetVariable( const char* inVarName )
{
	double*	foundVar = mVariables.find( inVarName );
	if (foundVar == NULL)
	{
		mVariables.add( inVarName, mValStack.back() );
		mVariableSet.insert( std::string(inVarName) );
	}
	else
	{
		*foundVar = mValStack.back();
	}
}

/*!
	@function	IsIdentifierChar
	@abstract	Test whether this character could be part of an identifier.
	@discussion	This code must be kept in agreement with the identifierLaterChar rule of the
				calculator grammar.
*/
static bool IsIdentifierChar( char inChar )
{
	unsigned char uChar = static_cast<unsigned char>( inChar );
	bool isGood = true;
	
	if ( (uChar <= ' ') or (strchr("-+=/*^,.;()", inChar )) )
	{
		isGood = false;
	}
	
	return isGood;
}

/*!
	@function	IsIdentifierStartChar
	@abstract	Test whether this character could be the first character of an identifier.
	@discussion	This code must be kept in agreement with the identifierFirstChar rule of the
				calculator grammar.
*/
static bool IsIdentifierStartChar( char inChar )
{
	bool isGood = IsIdentifierChar( inChar );
	
	if ( (inChar == '#') or ((inChar >= '0') and (inChar < '9')) )
	{
		isGood = false;
	}
	
	return isGood;
}

/*!
	@function	FixFormalParams
	@abstract	Prefix each formal parameter by an #, so it cannot be
				confused with a normal variable.
*/
static void FixFormalParams( FuncDef& ioDef )
{
	std::string&	rightHandSide( ioDef.second );
	
	for (std::string& theParam : ioDef.first)
	{
		std::string::size_type	startOff = 0;
		std::string::size_type	foundOff, identStart, identEnd;
		
		while ( (foundOff = rightHandSide.find( theParam, startOff )) !=
			std::string::npos )
		{
			// We found a substring that looks like the formal parameter, but we
			// must be sure that it is not just part of some other identifier.
			identStart = foundOff;
			identEnd = identStart + theParam.size();
			if ( (identEnd == rightHandSide.size()) or
				(not IsIdentifierChar(rightHandSide[identEnd])) )
			{
				// Looking to the left is trickier, e.g., while 12x is not an
				// identifier, A12x is.
				// First, go left until we either hit a character that is not
				// an identifier character, or we run out of text.
				while ( (identStart > 0) and
					IsIdentifierChar(rightHandSide[ identStart - 1 ]) )
				{
					--identStart;
				}
				// Now, if the character we're looking at could not be the
				// first letter of an identifier, go right.
				while (not IsIdentifierStartChar( rightHandSide[ identStart ]))
				{
					++identStart;
				}
				if (identStart == foundOff)
				{
					rightHandSide.insert( foundOff, 1, kParamPrefixChar );
					foundOff += 1;
				}
			}
			startOff = foundOff + 1;
		}
		
		theParam.insert( 0, 1, kParamPrefixChar );
	}
}


void	SCalcState::SetFunc( const std::string& inFuncName,
								const StringVec& inFormalParams,
								const std::string& inValue )
{
	FuncDef	thePair( inFormalParams, inValue );
	FixFormalParams( thePair );
	FuncDef*	foundDef = mFuncDefs.find( inFuncName.c_str() );
	if (foundDef == NULL)
	{
		mFuncDefs.add( inFuncName.c_str(), thePair );
		mFuncDefSet.insert( inFuncName );
	}
	else
	{
		*foundDef = thePair;
	}
}

CFDictionaryRef		SCalcState::CopyVariables() const
{
	autoCFMutableDictionaryRef	theDict( ::CFDictionaryCreateMutable( NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
	ThrowIfCFFail_( theDict.get() );
	
	for (const std::string& theVar: mVariableSet)
	{
		const double*	foundVal = mVariables.find( theVar.c_str() );
		if (foundVal != NULL)
		{
			autoCFStringRef	cfVarName( UTF8ToCFString( theVar.c_str() ) );
			ThrowIfCFFail_( cfVarName.get() );
			autoCFNumberRef	cfValue( ::CFNumberCreate( NULL, kCFNumberDoubleType,
				foundVal ) );
			ThrowIfCFFail_( cfValue.get() );
			::CFDictionaryAddValue( theDict.get(), cfVarName.get(), cfValue.get() );
		}
	}
	
	return theDict.release();
}

/*!
	@function	UnfixParams
	@abstract	Remove # characters from names
*/
static void UnfixParams( std::string& ioParam )
{
	std::string::size_type	atLoc;
	
	while ((atLoc = ioParam.find( kParamPrefixChar )) != std::string::npos)
	{
		ioParam.erase( atLoc, 1 );
	}
}

CFDictionaryRef	SCalcState::CopyFuncDefs() const
{
	autoCFMutableDictionaryRef	theDict( ::CFDictionaryCreateMutable( NULL, 0,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
	ThrowIfCFFail_( theDict.get() );
	
	for (const std::string&	funcName : mFuncDefSet)
	{
		// Find the function definition.
		const FuncDef*	theFuncPtr = mFuncDefs.find( funcName.c_str() );
		if (theFuncPtr != NULL)
		{
			// Make a copy so we can change it
			FuncDef	theFunc( *theFuncPtr );
		
			// Get name as CFString
			autoCFStringRef	cfFuncName( UTF8ToCFString( funcName.c_str() ) );
			ThrowIfCFFail_( cfFuncName.get() );
			
			// Create an emtpy array
			autoCFMutableArrayRef	theArray( ::CFArrayCreateMutable(
				NULL, 0, &kCFTypeArrayCallBacks ) );
			ThrowIfCFFail_( theArray.get() );

			// Put the RHS in the array
			UnfixParams( theFunc.second );
			autoCFStringRef	cfFuncDef( UTF8ToCFString( theFunc.second.c_str() ) );
			ThrowIfCFFail_( cfFuncDef.get() );
			::CFArrayAppendValue( theArray.get(), cfFuncDef.get() );
			
			for (std::string& param : theFunc.first)
			{
				UnfixParams( param );
				autoCFStringRef	cfParam( UTF8ToCFString( param.c_str() ) );
				ThrowIfCFFail_( cfParam.get() );
				::CFArrayAppendValue( theArray.get(), cfParam.get() );
			}
			
			::CFDictionarySetValue( theDict.get(), cfFuncName.get(),
				theArray.get() );
		}
	}
	
	return theDict.release();
}

static void VarSetter( const void *key, const void *value, void *context )
{
	SCalcState*	me = static_cast<SCalcState*>( context );
	CFStringRef	theKey = static_cast<CFStringRef>( key );
	CFNumberRef	theValueRef = static_cast<CFNumberRef>( value );
	
	std::string	keyStr( CFStringToUTF8( theKey ) );
	double	theValue;
	::CFNumberGetValue( theValueRef, kCFNumberDoubleType, &theValue );
	
	me->mValStack.push_back( theValue );
	me->SetVariable( keyStr.c_str() );
	me->mValStack.pop_back();
}

void	SCalcState::SetVariables( CFDictionaryRef inDict )
{
	::CFDictionaryApplyFunction( inDict, VarSetter, this );
}

static void FuncSetter( const void *key, const void *value, void *context )
{
	SCalcState*	me = static_cast<SCalcState*>( context );
	CFStringRef	theKey = static_cast<CFStringRef>( key );
	CFArrayRef	theValueRef = static_cast<CFArrayRef>( value );
	std::string	keyStr( CFStringToUTF8( theKey ) );
	CFIndex	arraySize = ::CFArrayGetCount( theValueRef );
	CFStringRef	funcDefRef = reinterpret_cast<CFStringRef>(
		::CFArrayGetValueAtIndex( theValueRef, 0 ) );
	FuncDef	theFunc;
	theFunc.second = CFStringToUTF8( funcDefRef );
	for (int i = 1; i < arraySize; ++i)
	{
		std::string	param( CFStringToUTF8( reinterpret_cast<CFStringRef>(
			::CFArrayGetValueAtIndex( theValueRef, i ) ) ) );
		theFunc.first.push_back( param );
	}
	
	me->SetFunc( keyStr, theFunc.first, theFunc.second );
}

void	SCalcState::SetFuncDefs( CFDictionaryRef inDict )
{
	::CFDictionaryApplyFunction( inDict, FuncSetter, this );
}

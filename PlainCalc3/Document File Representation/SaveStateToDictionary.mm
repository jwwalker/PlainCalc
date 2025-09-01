//  SaveStateToDictionary.m
//  PlainCalc3
//
//  Created by James Walker on 8/24/25.
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

#import "SaveStateToDictionary.h"

#import "SCalcState.hpp"

/*!
	@function	SaveStateToDictionary
	
	@abstract	Save the user-defined variables and functions of a calculator state to a dictionary.
				Specifically, the result is 3 dictionaries: the variables, the version 2 function data,
				and the version 3 function data.
*/
NSArray<NSDictionary*>* _Nonnull	SaveStateToDictionary( const SCalcState& inState )
{
	// Variables
	NSMutableDictionary* vars = [NSMutableDictionary dictionary];
	for (const auto& [name, value] : inState.variables)
	{
		vars[ @(name.c_str()) ] = @(value);
	}
	
	// Functions
	NSMutableDictionary* funcs = [NSMutableDictionary dictionary];
	NSMutableDictionary* funcsV3 = [NSMutableDictionary dictionary];
	for (const auto& [name, funcDef] : inState.userFunctions)
	{
		const StringVec& params( std::get<StringVec>( funcDef ) );
		const std::string& rhsString( std::get<std::string>( funcDef ) );
		const autoASTNode& treeNode( std::get<autoASTNode>( funcDef ) );
		autoCFDictionaryRef treeDictCF( treeNode->ToDictionary() );
		NSDictionary* treeDict = (__bridge NSDictionary*) treeDictCF.get();
		NSMutableArray* workArray = [NSMutableArray array];
		[workArray addObject: @(rhsString.c_str()) ];
		for (const auto& oneParam : params)
		{
			[workArray addObject: @(oneParam.c_str())];
		}
		funcs[ @(name.c_str()) ] = workArray;
		funcsV3[ @(name.c_str()) ] = treeDict;
	}
	
	NSArray<NSDictionary*>* result = @[
		vars,
		funcs,
		funcsV3
	];
	return result;
}

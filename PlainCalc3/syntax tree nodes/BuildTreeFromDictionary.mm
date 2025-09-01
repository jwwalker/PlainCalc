//  BuildTreeFromDictionary.cpp
//  PlainCalc3
//
//  Created by James Walker on 8/23/25.
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

#import "BuildTreeFromDictionary.hpp"

#import "BasicMath.hpp"
#import "BinaryFuncNode.hpp"
#import "Built-ins.hpp"
#import "IfNode.hpp"
#import "IndexVariableNode.hpp"
#import "IterationNode.hpp"
#import "Lookup.hpp"
#import "NaryFuncNode.hpp"
#import "NumberNode.hpp"
#import "ParameterIndexNode.hpp"
#import "UnaryFuncNode.hpp"
#import "UserFuncNode.hpp"

#import <Foundation/Foundation.h>
#import <math.h>
#import <assert.h>

using TreeMaker = autoASTNode (*)( NSDictionary* dict );

static autoASTNode BinaryFuncMaker( NSDictionary* dict )
{
	NSString* name = dict[@"name"];
	std::string nameC( name.UTF8String );
	BinaryFunc theFunc = nullptr;
	std::optional<BinaryFunc> maybeFunc = BuiltInBinarySyms().Lookup( nameC );
	if (maybeFunc.has_value())
	{
		theFunc = maybeFunc.value();
	}
	else if (nameC == "Plus")
	{
		theFunc = Plus;
	}
	else if (nameC == "Minus")
	{
		theFunc = Minus;
	}
	else if (nameC == "Multiply")
	{
		theFunc = Multiply;
	}
	else if (nameC == "Divide")
	{
		theFunc = Divide;
	}
	else if (nameC == "^")
	{
		theFunc = ::pow;
	}
	assert( theFunc != nullptr );
	NSDictionary* param1 = dict[@"param1"];
	autoASTNode p1tree = BuildTreeFromDictionary( param1 );
	NSDictionary* param2 = dict[@"param2"];
	autoASTNode p2tree = BuildTreeFromDictionary( param2 );
	autoASTNode resultTree( new BinaryFuncNode( theFunc, p1tree, p2tree ) );
	
	return resultTree;
}

static autoASTNode IfMaker( NSDictionary* dict )
{
	NSDictionary* test = dict[@"test"];
	autoASTNode testTree = BuildTreeFromDictionary( test );
	NSDictionary* param1 = dict[@"param1"];
	autoASTNode p1tree = BuildTreeFromDictionary( param1 );
	NSDictionary* param2 = dict[@"param2"];
	autoASTNode p2tree = BuildTreeFromDictionary( param2 );
	autoASTNode resultTree( new IfNode( testTree, p1tree, p2tree ) );
	
	return resultTree;
}

static autoASTNode NaryMaker( NSDictionary* dict )
{
	NSString* name = dict[@"name"];
	NaryFunc func = BuiltInNarySyms().at( name.UTF8String );
	std::vector< autoASTNode > argVec;
	NSArray* argArray = dict[@"args"];
	argVec.reserve( argArray.count );
	for (NSDictionary* argDict in argArray)
	{
		autoASTNode argNode = BuildTreeFromDictionary( argDict );
		argVec.push_back( argNode );
	}
	autoASTNode resultTree( new NaryFuncNode( func, argVec ) );
	
	return resultTree;
}

static autoASTNode NumberMaker( NSDictionary* dict )
{
	NSNumber* valueNum = dict[@"value"];
	autoASTNode resultTree( new NumberNode( valueNum.doubleValue ) );
	
	return resultTree;
}

static autoASTNode ParameterIndexMaker( NSDictionary* dict )
{
	NSNumber* valueNum = dict[@"index"];
	autoASTNode resultTree( new ParameterIndexNode( valueNum.unsignedIntValue ) );
	
	return resultTree;
}

static autoASTNode UnaryFuncMaker( NSDictionary* dict )
{
	NSString* name = dict[@"name"];
	UnaryFunc func = [name isEqualToString: @"Negate"]?
		Negate: BuiltInUnarySyms().at( name.UTF8String );
	NSDictionary* param1 = dict[@"param"];
	autoASTNode ptree = BuildTreeFromDictionary( param1 );
	autoASTNode resultTree( new UnaryFuncNode( func, ptree ) );
	
	return resultTree;
}

static autoASTNode UserFuncMaker( NSDictionary* dict )
{
	NSString* name = dict[@"name"];
	std::vector< autoASTNode > argVec;
	NSArray* argArray = dict[@"args"];
	argVec.reserve( argArray.count );
	for (NSDictionary* argDict in argArray)
	{
		autoASTNode argNode = BuildTreeFromDictionary( argDict );
		argVec.push_back( argNode );
	}
	autoASTNode resultTree( new UserFuncNode( name.UTF8String, argVec ) );
	
	return resultTree;
}

static autoASTNode IndexVariableMaker( NSDictionary* dict )
{
	NSString* name = dict[@"name"];
	autoASTNode resultTree( new IndexVariableNode( name.UTF8String ) );
	return resultTree;
}

static autoASTNode IterationNodeMaker( NSDictionary* dict )
{
	NSString* variableName = dict[@"variable"];
	IterationKind kind = (IterationKind) [dict[@"subtype"] integerValue];
	NSDictionary* start = dict[@"start"];
	autoASTNode startTree = BuildTreeFromDictionary( start );
	NSDictionary* end = dict[@"end"];
	autoASTNode endTree = BuildTreeFromDictionary( end );
	NSDictionary* content = dict[@"content"];
	autoASTNode contentTree = BuildTreeFromDictionary( content );
	autoASTNode resultTree( new IterationNode( kind, variableName.UTF8String,
		startTree, endTree, contentTree ) );
	return resultTree;
}

autoASTNode BuildTreeFromDictionary( NSDictionary* dict )
{
	autoASTNode resultTree;
	std::string kindStr( [dict[@"kind"] UTF8String] );
	
	static std::map<std::string, TreeMaker > sMakerMap
	{
		{ "BinaryFunc", BinaryFuncMaker },
		{ "If", IfMaker },
		{ "NaryFunc", NaryMaker },
		{ "Number", NumberMaker },
		{ "ParameterIndex", ParameterIndexMaker },
		{ "UnaryFunc", UnaryFuncMaker },
		{ "UserFunc", UserFuncMaker },
		{ "IndexVariable", IndexVariableMaker },
		{ "IterationNode", IterationNodeMaker },
	};
	
	std::optional<TreeMaker> maker( Lookup( kindStr, sMakerMap ) );
	
	if (maker.has_value())
	{
		resultTree = maker.value()( dict );
	}
	else
	{
		assert( "unimplemented tree node maker" );
	}
		
	return resultTree;
}

autoASTNode	BuildTreeFromDictionary( const autoCFDictionaryRef& dict )
{
	return BuildTreeFromDictionary( (__bridge NSDictionary*) dict.get() );
}

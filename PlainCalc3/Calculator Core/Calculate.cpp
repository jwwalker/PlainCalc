//  Calculate.cpp
//  ParserPlay
//
//  Created by James Walker on 7/26/25.
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

#import "Calculate.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import <boost/parser/parser.hpp>
#pragma clang diagnostic pop

#import "BasicMath.hpp"
#import "Built-ins.hpp"
#import "DoAssign.hpp"
#import "DoBinaryOperator.hpp"
#import "DoCheckForUserFunc.hpp"
#import "DoEvaluateBinary.hpp"
#import "DoEvaluateIteration.hpp"
#import "DoEvaluateVariable.hpp"
#import "DoEvaluateNary.hpp"
#import "DoEvaluateUnary.hpp"
#import "DoEvaluateUserFunc.hpp"
#import "DoGetAssignableIdentifier.hpp"
#import "DoIf.hpp"
#import "DoUserFuncDefine.hpp"
#import "DoGetIndexVariable.hpp"
#import "DoGetUserFuncName.hpp"
#import "DoGetUserFuncParam.hpp"
#import "DoNegate.hpp"
#import "DoPushFuncName.hpp"
#import "DoPushNumber.hpp"
#import "FuncArgCount.hpp"
#import "GetStackSize.hpp"
#import "UTF8toUTF32.hpp"


#import <iostream>
#import <string>
#import <vector>
#import <sstream>
#import <math.h>

namespace bp = ::boost::parser;

//MARK: Unsigned numbers

bp::rule< struct udouble, double > udouble = "udouble";

auto const udouble_def = bp::double_ -
					(bp::char_("+-") >> bp::double_);

BOOST_PARSER_DEFINE_RULES( udouble );

//MARK: identifiers

bp::rule< struct identifierFirst, char32_t > identifierFirst = "identifierFirst";
bp::rule< struct identifierLater, char32_t > identifierLater = "identifierLater";
bp::rule< struct identifier, std::u32string > identifier = "identifier";

auto const identifierFirst_def =
						identifierLater - bp::digit;

auto const identifierLater_def =
						bp::char_ -
						(
							bp::control
							|
							bp::ws
							|
							bp::char_( "-+=/*^,.;()#" )
						);

auto const identifier_def =
						bp::lexeme[ identifierFirst >> *(identifierLater) ];


BOOST_PARSER_DEFINE_RULES( identifierFirst, identifierLater, identifier );

//MARK: mathematical rules

/*!
	Roughly speaking:
	
	expression = additive combination of terms
	
	term = multiplicative combination of powers
	
	power = exponential combination of factors
	
	factor = literal number, or parenthetical expression, or function, or variable
*/

bp::rule< struct expression, double > expression = "expression";
bp::rule< struct expressionNA > expressionNA = "expression";

bp::rule< struct term, double > term = "term";
bp::rule< struct power, double > power = "factor or power";
bp::rule< struct factor, double > factor = "factor";

// This is a rule just to give better error messages
bp::rule< struct moreArgs, std::vector<double> > moreArgs = "more function arguments";
bp::rule< struct indexVar > indexVar = "iteration index variable";

auto const expression_def =
						(
							term
						|	('-' > term)[ DoNegate() ] // unary minus
						)
						>>
						*(
							('+' > term)[ DoBinaryOperator( Plus ) ]
						|	('-' > term)[ DoBinaryOperator( Minus ) ]
						);

auto const expressionNA_def = expression_def;

auto const indexVar_def = identifier[ DoGetIndexVariable() ];

auto const term_def = power >>
	(
		*(
			// Here is where we get multiplication by juxtaposition
			power[ DoBinaryOperator( Multiply ) ]
			
		|	('*' > power)[ DoBinaryOperator( Multiply ) ]
		|	('/' > power)[ DoBinaryOperator( Divide ) ]
		)
	);


auto const power_def = factor >>
					-(
						('^' > power)[ DoBinaryOperator( ::pow ) ]
					);

auto const moreArgs_def =
						bp::repeat( FuncArgCount() )[ ',' > expression ];

auto const factor_def =
						// literal nonnegative hexadecimal number
						// (we must check for hex before udouble, otherwise
						// something like 0x12 could be interpreted as 0
						// times a variable x12.
						(bp::lit("0x") > (bp::hex >> bp::eps)[ DoPushNumber() ])

						// literal nonnegative number
						|	udouble[ DoPushNumber() ]
						
						// parenthesized expression
						|	'(' > expression > ')'
						
						// unary function
						// 		Note 1: we do not allow a space between the
						//			function name and the left parenthesis
						// 		Note 2: we do NOT want an expectation point,
						//			i.e., > instead of >>, between the name
						//			and the parenthesis, because if we have
						//			matched the unary function "atan", we
						//			might actually be getting the binary
						//			function "atan2".
						|	( bp::lexeme[ BuiltInUnarySyms() >> '(' ][ DoPushFuncName() ] >
								expression >
								')'
							) [ DoEvaluateUnary() ]
						
						// binary function
						|	( bp::lexeme[ BuiltInBinarySyms() >> '(' ][ DoPushFuncName() ] >
								expression > ',' > expression >
								')'
							) [ DoEvaluateBinary() ]
						
						// n-ary function (2 or more arguments)
						|	( bp::omit[ bp::lexeme[ BuiltInNarySyms() >> '(' ] ][ DoPushFuncName() ] >
								expression > ',' >
								(expression % ',') > ')'
							) [ DoEvaluateNary() ]
						
						// user-defined function
						| 	(
								bp::omit[ bp::lexeme[ identifier[ DoCheckForUserFunc() ] >
								'(' ] ] >
								expression > moreArgs > ')'
							) [ DoEvaluateUserFunc() ]
						
						// iteration function (∑ or ∏)
						|	(
								bp::omit[ bp::lexeme[ BuiltInIterationSyms() >> '(' ] ][ DoPushFuncName() ] >
								indexVar > ',' >	// index variable
								expressionNA > ',' >	// start value
								expressionNA > ',' >	// end value
								expression >		// expression being summed/multiplied
								')'
							) [ DoEvaluateIteration() ]
						
						// If operator
						//		Note: if I don't omit the attribute on a couple
						//		of the expressions, then the rule does not
						//		compile, because the rule is declared to have
						//		just a double as its attribute.
						//		The reason for attaching actions to eps is so
						//		that expected ',' error messages will not
						//		mention the action.
						|	(
								bp::lit("if(") >
								expression >
								bp::lit(',') >> bp::eps[ DoIfAtFirstComma() ] >
								expressionNA >
								bp::lit(',') >
								expressionNA >
								bp::lit(')') >> bp::eps[ DoIfFinish() ]
							)

						// constant or variable
						|	identifier[ DoEvaluateVariable() ];

BOOST_PARSER_DEFINE_RULES( expression, expressionNA, term, factor, power,
	moreArgs, indexVar );

//MARK: top-level types of calculations

bp::rule< struct assignment, double > assignment = "assignment";
bp::rule< struct funcdef > funcdef = "function definition";
bp::rule< struct expressionStatement, double > expressionStatement = "expression statement";

// These parts were made into rules to get better error messages.
bp::rule< struct funcName > funcName = "function name";
bp::rule< struct funcParams > funcParams = "function parameters";
bp::rule< struct varName > varName = "variable name";

auto const expressionStatement_def =
						bp::eps > expression >> bp::eoi;

auto const varName_def =
						identifier[ DoGetAssignableIdentifier() ];

auto const assignment_def =
						bp::eps > 
						varName >
						'=' > expression >> bp::eoi[ DoAssign() ];

auto const funcName_def =
						bp::lexeme[ identifier[ DoGetUserFuncName() ] > '(' ];

auto const funcParams_def =
						(identifier[ DoGetUserFuncParam() ] % ',');

auto const funcdef_def =
						(
							bp::eps >
							funcName > funcParams >
							')' > '=' >>
							bp::eps[ DoUserFuncDefine( false ) ] >
							expression >> bp::eoi
						)[ DoUserFuncDefine( true ) ];

BOOST_PARSER_DEFINE_RULES( assignment, funcdef, expressionStatement,
	funcName, funcParams, varName );

//MARK: -

// Figure out whether we are looking at a calculation, an assignment to a
// variable, or a definition of a function.
static CalcType DeduceCalcType( const std::string& inText )
{
	CalcType theType = CalcType::expression;
	
	std::string::size_type equalOffset = inText.find( '=' );
	if (equalOffset != std::string::npos)
	{
		std::string::size_type parenOffset = inText.find( '(' );
		if (parenOffset < equalOffset)
		{
			theType = CalcType::functionDefinition;
		}
		else
		{
			theType = CalcType::variableAssignment;
		}
	}
	
	return theType;
}

#define DEBUG_TRACING	0

#if DEBUG_TRACING
	#define DEBUG_TRACING_OPTION	, boost::parser::trace::on
#else
	#define DEBUG_TRACING_OPTION
#endif

/*!
	@function	Calculate
	
	@abstract	Parse and execute a calculator statement.
	
	@param		inText		A line of input text.
	@param		ioState		A state object that may be used and modified in the course
							of the calculation.
	@result		The result of the calculation.
*/
CalcResult	Calculate( const std::string& inText, SCalcState& ioState )
{
	SaveStackAddress();
	CalcResult returnedVariant( "No calc yet" );
	ioState.ClearTemporaries();
	CalcType calcType = DeduceCalcType( inText );
	std::u32string text32( UTF8toUTF32( inText ) );
	
	std::ostringstream errors;
	bp::stream_error_handler errorHandler( "", errors );
	bool didParse = false;
	
	switch (calcType)
	{
		case CalcType::unknown: // should never happen
			break;
			
		case CalcType::expression:
			didParse = static_cast<bool>( bp::parse( text32,
				bp::with_error_handler(
				bp::with_globals(expressionStatement, ioState), errorHandler),
				bp::ws
				DEBUG_TRACING_OPTION
				) );
			if (didParse)
			{
				autoASTNode resultNode( ioState.valStack.top() );
				std::optional<double> resultVal = resultNode->Evaluate( ioState );
				if (ioState.interruptCode != CalcInterruptCode::none)
				{
					returnedVariant = ioState.interruptCode;
				}
				else if (resultVal.has_value())
				{
					returnedVariant = resultVal.value();
					ioState.variables["last"] = resultVal.value();
				}
			}
			break;
		
		case CalcType::variableAssignment:
			didParse = static_cast<bool>( bp::parse( text32,
				bp::with_error_handler( bp::with_globals(assignment, ioState),
				errorHandler ), bp::ws
				DEBUG_TRACING_OPTION
				) );
			if (didParse)
			{
				autoASTNode resultNode( ioState.valStack.top() );
				std::optional<double> resultVal = resultNode->Evaluate( ioState );
				if (ioState.interruptCode != CalcInterruptCode::none)
				{
					returnedVariant = ioState.interruptCode;
				}
				else if (resultVal.has_value())
				{
					returnedVariant = resultVal.value();
					ioState.variables["last"] = resultVal.value();
				}
			}
			break;
		
		case CalcType::functionDefinition:
			didParse = static_cast<bool>( bp::parse( text32,
				bp::with_error_handler( bp::with_globals(funcdef, ioState),
				errorHandler ), bp::ws
				DEBUG_TRACING_OPTION
				) );
			if (didParse)
			{
				returnedVariant = DefinedFunc( ioState.leftIdentifier,
					ioState.preexistingUserFunc );
			}
			else
			{
				ioState.userFunctions.erase( ioState.leftIdentifier );
			}
			break;
	}
	
	if (not didParse)
	{
		returnedVariant = errors.str();
	}
	
	return returnedVariant;
}

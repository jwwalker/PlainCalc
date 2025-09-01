//  CalcTests.m
//  CalcTests
//
//  Created by James Walker on 8/20/25.
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

#import <XCTest/XCTest.h>

#import "BuildTreeFromDictionary.hpp"
#import "Calculate.hpp"
#import "SCalcState.hpp"

#import <math.h>
#import <iostream>

@interface CalcTests : XCTestCase

@end

@implementation CalcTests

- (void)setUp {
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
}

- (void) testPlusAndMinus
{
	SCalcState state;
	auto result = Calculate( "2+3", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 5.0 );
	result = Calculate( "-2 + 3", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 1.0 );
	result = Calculate( "2 - 3", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), -1.0 );
	result = Calculate( "2-3", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), -1.0 );
	result = Calculate( "1 + 2 + 3 +4 +5", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 15.0 );
}

- (void) testDoNegate
{
	SCalcState state;
	auto result = Calculate( "-(2+7)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), -9.0 );
	result = Calculate( "x=5", state );
	result = Calculate( "-x", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), -5.0 );
}

- (void) testHexadecimalLiterals
{
	SCalcState state;
	auto result = Calculate( "0xabc124 + 0x1FE", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 11256610.0 );
}

- (void) testTimesAndDiv
{
	SCalcState state;
	auto result = Calculate( "2*3", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 6.0 );
	result = Calculate( "6 / 2", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 3.0 );
}

- (void) testPlusAndTimesPrecedence
{
	SCalcState state;
	auto result = Calculate( "2 + 3 * 4", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 14.0 );
	result = Calculate( "2 * 3 + 4", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 10.0 );
}

- (void) testPowerRightAssociation
{
	SCalcState state;
	auto result = Calculate( "4^3^2", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 262144.0 );
}

- (void) testBuiltInFuncs
{
	SCalcState state;
	auto result = Calculate( "√(16)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	double val = std::get<double>( result );
	XCTAssert( fabs( 4.0 - val ) < 1.0e-10 );

	result = Calculate( "deg(atan2(sqrt(3), 1))", state );
	XCTAssert( std::holds_alternative<double>(result) );
	val = std::get<double>( result );
	XCTAssert( fabs( 60.0 - val ) < 1.0e-10 );
	
	// Make sure we don't get confused between log, log2, and log10.
	result = Calculate( "log(e^3)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	val = std::get<double>( result );
	XCTAssert( fabs( 3.0 - val ) < 1.0e-10 );
	result = Calculate( "log2(32768)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	val = std::get<double>( result );
	XCTAssert( fabs( 15.0 - val ) < 1.0e-10 );
	result = Calculate( "log10(32768)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	val = std::get<double>( result );
	XCTAssert( fabs( 4.51544993496 - val ) < 1.0e-10 );
}

- (void) testNaryBuiltInFunctions
{
	SCalcState state;
	auto result = Calculate( "Var(1,2,3,4,5,6)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( 2.91667 - std::get<double>( result ) ) < 1.0e-4 );
	
	result = Calculate( "SD(3,2,5,4,1,6)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( 1.70783 - std::get<double>( result ) ) < 1.0e-4 );
	
	result = Calculate( "GM(3,2,5,4,1,6)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( 2.9938 - std::get<double>( result ) ) < 1.0e-4 );
	
	result = Calculate( "HM(3,2,5,4,1,6)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( 2.44898 - std::get<double>( result ) ) < 1.0e-4 );
	
	result = Calculate( "median(3,2,5,4,1,6)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( 3.5 - std::get<double>( result ) ) < 1.0e-10 );
}

- (void) testAssigningVariables
{
	SCalcState state;
	auto result = Calculate( "x = 2 * 3 * 4", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 24.0 );
	result = Calculate( "x=22", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 22.0 );
	result = Calculate( "x - 2", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 20.0 );
	result = Calculate( "ynot = 5 * 6", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 30.0 );
	result = Calculate( "x - ynot", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), -8.0 );
	
	// At one time, the built in constant e prevented me from setting a
	// variable beginning with e, so make sure that doesn't happen.
	result = Calculate( "error = 43", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 43.0 );
	
	// Make sure I can't assign to a function name or constant
	result = Calculate( "e = 43", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	result = Calculate( "cos = 43", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	result = Calculate( "hypot = 43", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	result = Calculate( "SD = 43", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
}

- (void) testEvaluateBuiltInConstants
{
	SCalcState state;
	auto result = Calculate( "e", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( std::get<double>( result ) - 2.71828 ) < 0.00001 );
	result = Calculate( "π", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( std::get<double>( result ) - 3.14159265 ) < 0.00001 );
	result = Calculate( "2π π", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssert( fabs( std::get<double>( result ) - 19.7392088 ) < 0.00001 );
}

- (void) testUnicode
{
	SCalcState state;
	auto result = Calculate( "🎂✅ = 4 + 1", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 5.0 );
	
	result = Calculate( "3 + 🎂✅^2", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 28.0 );
	
	// Now to check the error location.
	result = Calculate( "3 + 🎂✅**2", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	std::string errMsg( std::get<std::string>( result ) );
	std::string::size_type colon1 = errMsg.find(':');
	std::string::size_type colon2 = errMsg.find(':', colon1+1);
	std::string errOffset( errMsg, colon1+1, colon2-colon1-1 );
	XCTAssertEqual( errOffset, "7" );
}

- (void) testJuxtaposition
{
	SCalcState state;
	auto result = Calculate( "5 6 7", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 210.0 );
	result = Calculate( "(1+2)(3+4)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 21.0 );
	result = Calculate( "cos(2pi)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 1.0 );
	
	// These should NOT be parsed as multiplication by juxtaposition
	result = Calculate( "5 -2", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 3.0 );
	result = Calculate( "5 +2", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 7.0 );
}

- (void) testErrorReport
{
	SCalcState state;
	auto result = Calculate( "120++1-2", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	std::string errMsg( std::get<std::string>( result ) );
	std::string expected( "1:4: error: Expected term here:\n120++1-2\n    ^\n" );
	XCTAssert( errMsg == expected );
}

- (void) testUserDefinedFunctions
{
	SCalcState state;
	// A valid definition
	auto result = Calculate( "myfunc( a, b, c ) = a^2 + 2b + c", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	
	// Valid use of function in an expression
	result = Calculate( "3 + myfunc(3, 5, (2+2)(3+3)) - 1", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 45.0 );
	
	// Using user functions within other user functions
	result = Calculate( "double( x ) = 2x", state );
	result = Calculate( "triple( x ) = 3 x", state );
	result = Calculate( "myfunc( double(triple(5)), double(2), triple(7) )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 929.0 );
	
	// Invalid use, wrong param count
	result = Calculate( "3 + myfunc(3, (2+2)(3+3)) - 1", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	
	// invalid use, no parentheses
	result = Calculate( "3 + myfunc - 1", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	
	// Invalid definition, formal param is a constant
	result = Calculate( "myfunc( a, e, c ) = a^2 + 2e + c", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	
	// Invalid definition, formal params not all distinct
	result = Calculate( "myfunc( a, b, a ) = a^2 + 2b + a", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	
	// If a formal parameter happens to be the same as a user-defined
	// variable, make sure we keep it as a formal parameter instead
	// of replacing it with the variable's value.
	result = Calculate( "x=5", state );
	result = Calculate( "g(x)= 2 x", state );
	result = Calculate( "g(3)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 6.0 );
	
	// If the RHS of a function contains a variable, it should use the value
	// of that variable when the function was defined, even if the variable
	// is changed later.
	// x is now 5, so this should multiply y times 5
	result = Calculate( "h(y)= y x", state );
	result = Calculate( "x=1", state );
	result = Calculate( "h(3)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 15.0 );
}

- (void) testNonsenseAtEndOfLine
{
	SCalcState state;
	auto result = Calculate( "2 * 4 xxxxx", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	
	result = Calculate( "y = 2 * 4 xxxxx", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
	
	result = Calculate( "f(x) = 2 * 4 xxxxx", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
}

- (void) testIfOperator
{
	SCalcState state;
	auto result = Calculate( "if( 3, 2*(4+5), (1+2)(3+4) )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 18.0 );
	
	result = Calculate( "if( -33, 2*(4+5), (1+2)(3+4) )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 21.0 );
	
	result = Calculate( "if( -33, 2*if(1, 2, 3), (1+2)if(0, 2, 3) )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 9.0 );
	
	result = Calculate( "if( 3, 2*if(1, 2, 3), (1+2)if(0, 2, 3) )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 4.0 );
	
	// Now a few improper uses
	result = Calculate( "if( 3, 4 )", state );
	XCTAssert( std::holds_alternative<std::string>(result) );

	result = Calculate( "if = 3", state );
	XCTAssert( std::holds_alternative<std::string>(result) );

	result = Calculate( "2*if + 3", state );
	XCTAssert( std::holds_alternative<std::string>(result) );
}

- (void) testFactorial
{
	SCalcState state;
	auto result = Calculate( "!(n) = if(n, n !(n-1), 1)", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	
	result = Calculate( "!(9)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 362880.0 );
}

- (void) testRedefineFuncAsVariable
{
	SCalcState state;
	auto result = Calculate( "e1(x)=2x", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	result = Calculate( "e1(3)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 6.0 );
	
	// Redefine the function as a variable
	result = Calculate( "e1=5", state );
	XCTAssert( std::holds_alternative<double>(result) );
	result = Calculate( "e1(3)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 15.0 );
	
	// Back to a function
	result = Calculate( "e1(x)=2x", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	result = Calculate( "e1(3)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 6.0 );
}

- (void) testStackLimit
{
	SCalcState state;
	auto result = Calculate( "f(n) = 1 + f(n)", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	result = Calculate( "f(1)", state );
	XCTAssert( std::holds_alternative<CalcInterruptCode>(result) );
	XCTAssertEqual( std::get<CalcInterruptCode>( result ),
		CalcInterruptCode::stackLimit );
}

- (void) testdictionaryRepresentation
{
	SCalcState state;
	auto result = Calculate( "myfunc( a, b, c ) = a^2 + 2b + c", state );
	result = Calculate( "double( x ) = 2x", state );
	result = Calculate( "triple( x ) = 3 x", state );
	result = Calculate( "f(a, b, c) = -b + 7+ myfunc( double(a)+1, 2b, exp(a) * triple(c))^2", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	FuncDef& def( state.userFunctions[ "f" ] );
	autoASTNode rhs( std::get<autoASTNode>( def ) );
	XCTAssertEqual( rhs->Count(), 20 );
	autoCFDictionaryRef theDict( rhs->ToDictionary() );
	autoASTNode rebuilt( BuildTreeFromDictionary( theDict ) );
	XCTAssert( *rebuilt == *rhs );
}

- (void) testIteration
{
	SCalcState state;
	auto result = Calculate( "∏( i, 1, 10, i )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 3628800.0 );
	result = Calculate( "∏( i, 1, 10, 1+1/i )", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 11.0 );
	result = Calculate( "f(n) = ∑( i, 1, n, i^2 )", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	result = Calculate( "f(10)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 385.0 );
	result = Calculate( "h(n,k) = ∑(i, 1, n, ∑( j, 0, k, i+j))", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	result = Calculate( "h(4,5)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 120.0 );
	result = Calculate( "g(n,k) = ∑(i, 1, n, ∏( j, 0, k, i+j))", state );
	XCTAssert( std::holds_alternative<DefinedFunc>(result) );
	result = Calculate( "g(4,5)", state );
	XCTAssert( std::holds_alternative<double>(result) );
	XCTAssertEqual( std::get<double>( result ), 86400.0 );
}

- (void)testPerformanceExample
{
    // This is an example of a performance test case.
    [self measureBlock:^{
		SCalcState state;
		auto result = Calculate( "d( n, k ) = if( (n-k)k, d(n-1,k) + "
			"d(n-1,k-1), 1 ) ", state );
		XCTAssert( std::holds_alternative<DefinedFunc>(result) );
		result = Calculate( "d(17,8)", state );
		XCTAssert( std::holds_alternative<double>(result) );
        // Put the code you want to measure the time of here.
    }];
}


@end

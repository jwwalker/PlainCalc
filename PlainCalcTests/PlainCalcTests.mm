//  PlainCalcTests.m
//  PlainCalcTests
//
//  Created by James Walker on 3/27/23.
//  
//

#import <XCTest/XCTest.h>

#import "CalcStateFunctions.h"
#import "ParseCalcLine.h"

@interface PlainCalcTests : XCTestCase

@end

@implementation PlainCalcTests
{
	CalcState _calculator;
}

- (void)setUp
{
    // Put setup code here. This method is called before the invocation of each test method in the class.
    _calculator = CreateCalcState();;
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    DisposeCalcState( _calculator );
    _calculator = nullptr;
}

- (void)testSimplest
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "11+ 2", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 13.0 );
}

- (void)testPower
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "2^7", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 128.0 );
}


- (void)testMoreArithmetic
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "(4+ 2^(10-3))(54/6 -1/8)", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 1171.5 );
}

- (void)testBuiltins
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "sin(π/6) + (cos(pi/6))^2", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 1.25 );
}

- (void)testDefineVariable
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "xy22 = 4+2^10", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 1028.0 );
	result = ParseCalcLine( "xy22 /4", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 257.0 );
}

- (void)testDefineFunction
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "FtoC( fdeg ) = (fdeg-32) 5/9", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_DefinedFunction );

	result = ParseCalcLine( "FtoC(113)", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 45.0 );
}


/*- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}*/

@end

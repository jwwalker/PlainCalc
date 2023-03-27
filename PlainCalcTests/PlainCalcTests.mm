//  PlainCalcTests.m
//  PlainCalcTests
//
//  Created by James Walker on 3/27/23.
//  
//

#import <XCTest/XCTest.h>

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

- (void)testExample
{
	double computedValue;
	long stopOffset;
	ECalcResult result = ParseCalcLine( "11+ 2^10", _calculator,
		&computedValue, &stopOffset );
	XCTAssertEqual( result, kCalcResult_Calculated );
	XCTAssertEqual( computedValue, 1035.0 );
 }

/*- (void)testPerformanceExample {
    // This is an example of a performance test case.
    [self measureBlock:^{
        // Put the code you want to measure the time of here.
    }];
}*/

@end

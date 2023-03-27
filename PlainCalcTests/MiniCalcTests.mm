//  MiniCalcTests.h
//  PlainCalc2
//
//  Created by James Walker on 3/27/23.
//  
//
#import <XCTest/XCTest.h>

#import "CalcStateFunctions.h"
#import "MiniCalc.hpp"

@interface MiniCalcTests : XCTestCase

@end


@implementation MiniCalcTests
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
	XCTAssert( _calculator != nullptr );
	double result = MiniCalc( "13 + 9 + 11", _calculator );
	XCTAssertEqual( result, 33.0 );
}

@end


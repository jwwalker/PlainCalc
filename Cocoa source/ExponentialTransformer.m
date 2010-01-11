//
//  ExponentialTransformer.m
//  PlainCalc2
//
//  Created by James Walker on 1/10/10.
//  Copyright 2010 James W. Walker. All rights reserved.
//

#import "ExponentialTransformer.h"

#import <math.h>

@implementation ExponentialTransformer

+ (Class) transformedValueClass
{
	return [NSNumber class];
}

+ (BOOL)allowsReverseTransformation
{
	return NO;
}

- (id) initWithBase: (double) theBase
{
	if ( (self = [super init]) != nil )
	{
		mBase = theBase;
	}
	return self;
}

- (id) transformedValue: (id) value
{
	if (value == nil) return nil;
	
	double theResult;
	
	if ([value respondsToSelector: @selector(doubleValue)])
	{
		double theExponent = [value doubleValue];
		theResult = pow( mBase, theExponent );
	}
	else
	{
		[NSException raise: NSInternalInconsistencyException
                    format: @"Value (%@) does not respond to -floatValue.",
					[value class]];
	}

	return [NSNumber numberWithDouble: theResult];
}

@end

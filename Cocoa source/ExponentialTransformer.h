//
//  ExponentialTransformer.h
//  PlainCalc2
//
//  Created by James Walker on 1/10/10.
//  Copyright 2010 James W. Walker. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface ExponentialTransformer : NSValueTransformer
{
	double	mBase;
}

- (id) initWithBase: (double) theBase;

@end

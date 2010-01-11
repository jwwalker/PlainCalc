//
//  AppController.h
//  PlainCalc2
//
//  Created by James Walker on 12/31/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface AppController : NSObject
{
	NSDictionary*	mErrorColorAtt;
	NSDictionary*	mSuccessColorAtt;
	NSDictionary*	mNormalColorAtt;
	double			mCalcTimeout;
}

+ (AppController*) sharedController;

+ (NSDictionary*) normalAtts;

+ (NSDictionary*) errorAtts;

+ (NSDictionary*) successAtts;

+ (double) calcTimeout;

@end

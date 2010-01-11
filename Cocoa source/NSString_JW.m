//
//  NSString_JW.m
//  PlainCalc2
//
//  Created by James Walker on 1/10/10.
//  Copyright 2010 James W. Walker. All rights reserved.
//

#import "NSString_JW.h"


@implementation NSString(JW)

- (NSString*) stringByReplacingChar: (unichar) toReplace
			byChar: (unichar) replaceWith
{
	NSMutableString* resultStr = [[self mutableCopy] autorelease];
	
	NSString* toRep = [NSString stringWithCharacters: &toReplace
								length: 1 ];
	NSString* repWith = [NSString stringWithCharacters: &replaceWith
								length: 1 ];
	[resultStr replaceOccurrencesOfString: toRep
				withString: repWith
				options: 0
				range: NSMakeRange( 0, [self length] ) ];

	return resultStr;
}

@end

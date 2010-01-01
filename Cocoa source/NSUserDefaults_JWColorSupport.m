//
//  NSUserDefaults_JWColorSupport.m
//  PlainCalc2
//
//  Created by James Walker on 12/31/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import "NSUserDefaults_JWColorSupport.h"


@implementation NSUserDefaults(JWColorSupport)

- (void)jw_setColor:(NSColor *)aColor forKey:(NSString *)aKey
{
	NSData *theData=[NSArchiver archivedDataWithRootObject:aColor];
	[self setObject:theData forKey:aKey];
}

- (NSColor *)jw_colorForKey:(NSString *)aKey
{
	NSColor *theColor=nil;
	NSData *theData=[self dataForKey:aKey];
	if (theData != nil)
		 theColor=(NSColor *)[NSUnarchiver unarchiveObjectWithData:theData];
	return theColor;
}


@end

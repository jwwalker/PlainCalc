//
//  NSUserDefaults_JWColorSupport.h
//  PlainCalc2
//
//  Created by James Walker on 12/31/09.
//  Copyright 2009 James W. Walker. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface NSUserDefaults(JWColorSupport)

- (void)jw_setColor:(NSColor *)aColor forKey:(NSString *)aKey;

- (NSColor *)jw_colorForKey:(NSString *)aKey;

@end

//
//  BoyiaBridge.m
//  core
//
//  Created by yanbo on 2022/6/7.
//

#import <Foundation/Foundation.h>
#import "BoyiaBridge.h"

@implementation BoyiaBridge

+(NSString*)getAppRoot {
    NSBundle* coreBundle = [NSBundle bundleWithIdentifier:@"com.boyia.core"];
    NSString* appDir = [coreBundle pathForResource:@"metal" ofType:@"bundle"];
    NSBundle* appBundle = [NSBundle bundleWithPath:appDir];
    NSString* boyiaAppDir = [appBundle.bundlePath stringByAppendingString:@"/"];
    return boyiaAppDir;
}

@end

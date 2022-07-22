//
//  BoyiaBridge.m
//  core
//
//  Created by yanbo on 2022/6/7.
//

#import <Foundation/Foundation.h>
#import "BoyiaBridge.h"

@implementation BoyiaBridge

+(NSBundle*)getAppBundle {
    NSBundle* coreBundle = [NSBundle bundleWithIdentifier:@"com.boyia.core"];
    NSString* appDir = [coreBundle pathForResource:@"metal" ofType:@"bundle"];
    NSBundle* appBundle = [NSBundle bundleWithPath:appDir];
    return appBundle;
}

+(NSString*)getAppRoot {
//    NSBundle* coreBundle = [NSBundle bundleWithIdentifier:@"com.boyia.core"];
//    NSString* appDir = [coreBundle pathForResource:@"metal" ofType:@"bundle"];
//    NSBundle* appBundle = [NSBundle bundleWithPath:appDir];
    NSString* boyiaAppDir = [[BoyiaBridge getAppBundle].bundlePath stringByAppendingString:@"/"];
    return boyiaAppDir;
}

+(NSString*)getSSLCertPath {
    NSBundle* bundle = [BoyiaBridge getAppBundle];
    //return [bundle pathForResource:@"cert" ofType:@"cert"];
    NSString* boyiaCertPath = [bundle.bundlePath stringByAppendingString:@"/cert/boyia_server.cert"];
    return boyiaCertPath;
}

+(void)handleApi:(NSString*)apiName andParams:(NSString*)params {
    
}

@end

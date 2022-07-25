//
//  BoyiaBridge.m
//  core
//
//  Created by yanbo on 2022/6/7.
//

#import <Foundation/Foundation.h>
#import "BoyiaBridge.h"

static NSMutableDictionary* _apiMap = nil;

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

+(NSMutableDictionary*)apiMap {
    if (!_apiMap) {
        _apiMap = [NSMutableDictionary new];
    }
    
    return _apiMap;
}

+(void)registerApi:(NSString*)apiName creator:(BoyiaApiCreator)creator {
    [[BoyiaBridge apiMap]setObject:creator forKey:apiName];
}

+(void)handleApi:(NSString*)json callback:(id<BoyiaApiHandlerCB>)cb {
    NSMutableDictionary* map = [BoyiaBridge apiMap];
    NSData* data = [json dataUsingEncoding:NSUTF8StringEncoding];
    NSMutableDictionary* jsonObj = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingMutableContainers error:nil];
    
    NSString* method = jsonObj[@"api_method"];
    NSDictionary* params = jsonObj[@"api_params"];
    if (!method || !params) {
        return;
    }
    
    BoyiaApiCreator creator = map[method];
    [creator() handle:params callback:cb];
}

@end

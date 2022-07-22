//
//  BoyiaBridge.h
//  core
//
//  Created by yanbo on 2022/6/7.
//

#ifndef BoyiaBridge_h
#define BoyiaBridge_h

#import <Foundation/Foundation.h>

@interface BoyiaBridge : NSObject

+(NSString*)getAppRoot;
+(NSString*)getSSLCertPath;
+(void)handleApi:(NSString*)apiName andParams:(NSString*)params;

@end


#endif /* BoyiaBridge_h */

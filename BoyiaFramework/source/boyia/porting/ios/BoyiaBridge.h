//
//  BoyiaBridge.h
//  core
//
//  Created by yanbo on 2022/6/7.
//

#ifndef BoyiaBridge_h
#define BoyiaBridge_h

#import <Foundation/Foundation.h>

@protocol BoyiaApiHandlerCB <NSObject>
-(void)callback:(NSString*)json;
@end

@protocol BoyiaApiHandler <NSObject>
-(void)handle:(NSDictionary*)json callback:(id<BoyiaApiHandlerCB>)cb;
@end

//@protocol BoyiaApiCreator <NSObject>
//-(id<BoyiaApiHandler>)create;
//@end
typedef id<BoyiaApiHandler> (^BoyiaApiCreator)(void);

@interface BoyiaBridge : NSObject

+(NSString*)getAppRoot;
+(NSString*)getSSLCertPath;
+(void)registerApi:(NSString*)apiName creator:(BoyiaApiCreator)creator;
+(void)handleApi:(NSString*)params callback:(id<BoyiaApiHandlerCB>)cb;
@end


#endif /* BoyiaBridge_h */

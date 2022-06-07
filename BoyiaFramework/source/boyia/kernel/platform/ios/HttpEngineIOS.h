//
//  HttpEngineIOS.h
//  core
//
//  Created by yanbo on 2021/8/20.
//

#ifndef HttpEngineIOS_h
#define HttpEngineIOS_h

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, HttpMethod) {
    kHttpGet = 0,
    kHttpPost
};

@protocol HttpCallback <NSObject>

-(void)onProgress:(int64_t)current total:(int64_t)total;
// 回调网络数据
-(void)onDataReceive:(NSData*)data;
// 请求完成
-(void)onLoadFinished;
// 请求失败
-(void)onLoadError;

@end

// IOS http请求类
@interface HttpEngineIOS : NSObject

-(void)setData:(NSString*)data;
-(void)setData:(const char*)data size:(NSInteger)size;
-(void)loadUrl:(HttpMethod)method url:(NSString*) url headers:(NSDictionary*)headers callback:(id<HttpCallback>)cb;
-(void)loadUrlWithData:(HttpMethod)method url:(const char *)url headers:(NSDictionary*)headers callback:(id<HttpCallback>)cb;

@end

#endif /* HttpEngineIOS_h */

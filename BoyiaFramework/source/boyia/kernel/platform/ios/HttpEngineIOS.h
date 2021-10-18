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

// 回调网络数据
-(void)onDataReceive:(NSData*)data;
// 请求完成
-(void)onLoadFinished;

@end

// IOS http请求类
@interface HttpEngineIOS : NSObject

//-(void)setHttpCallback:(id<HttpCallback>)cb;
-(void)setData:(const char*)data size:(NSInteger)size;
-(void)loadUrl:(HttpMethod) method url: (NSString*) url callback:(id<HttpCallback>)cb;
-(void)loadUrlWithData: (HttpMethod) method url:(const char *)url callback:(id<HttpCallback>)cb;

@end

#endif /* HttpEngineIOS_h */

//
//  HttpEngineIOS.m
//  core
//
//  Created by yanbo on 2021/8/20.
//

#import <Foundation/Foundation.h>

#import "HttpEngineIOS.h"

// 实现请求代理
@interface HttpEngineIOS ()<NSURLSessionDataDelegate>

@property (nonatomic, assign) NSString* postData;
@property (strong) NSMutableData* receiveData;
@property (strong) id<HttpCallback> callback;

@end

@implementation HttpEngineIOS

@synthesize callback;

-(void)setHttpCallback:(id<HttpCallback>)cb {
    self.callback = cb;
}

-(void) loadUrl: (HttpMethod) method andUrl: (NSString*) url {
    self.receiveData = [NSMutableData new];
    NSURL *reqUrl = [NSURL URLWithString:url];
    // 初始化请求对象
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:reqUrl cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:15];
    // 设置HTTP的method
    switch (method) {
        case kHttpGet: {
            request.HTTPMethod = @"GET";
        } break;
        case kHttpPost: {
            request.HTTPMethod = @"POST";
            // 设置post数据
            request.HTTPBody = [self.postData dataUsingEncoding:NSUTF8StringEncoding];
        } break;
        default:
            break;
    }
    
    // 使用默认配置
    NSURLSessionConfiguration* config = [NSURLSessionConfiguration defaultSessionConfiguration];
    
    // 创建操作队列
    NSOperationQueue *queue = [NSOperationQueue new];
    
    // 创建Session, 设置代理
    NSURLSession* session = [NSURLSession sessionWithConfiguration:config delegate:self delegateQueue:queue];
    
    // 创建任务对象
    NSURLSessionTask* task = [session dataTaskWithRequest:request];
//    NSURLSessionTask *task = [session dataTaskWithRequest: request completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
//        // 此处网络请求成功
//        if (data && !error) {
//
//            NSString* json = [[NSString alloc] initWithData:data encoding:(NSUTF8StringEncoding)];
//
//            NSLog(@"Result data = %@",json);
//        }
//    }];
    
    // 启动任务
    [task resume];
}

-(void)loadUrlWithData: (HttpMethod) method andUrl:(const char *)url {
    NSString* nsUrl = [[NSString alloc] initWithUTF8String: url];
    [self loadUrl:method andUrl:nsUrl];
}

-(void)setData: (const char*)data {
    NSString* postData = [[NSString alloc] initWithUTF8String: data];
    self.postData = postData;
}

#pragma mark --NSURLSessionDownloadDelegate

// 接受网络数据
- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveData:(NSData *)data {
    [self.receiveData appendData:data];
    if (self.callback != nil) {
        [self.callback onDataReceive:data];
    }
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
didCompleteWithError:(nullable NSError *)error {
    NSString* json = [[NSString alloc] initWithData:self.receiveData encoding:(NSUTF8StringEncoding)];
    NSLog(@"Result New data = %@",json);
    if (self.callback != nil) {
        [self.callback onLoadFinished];
    }
}


@end

//
//  HttpEngineIOS.m
//  core
//
//  Created by yanbo on 2021/8/20.
//

#import <Foundation/Foundation.h>

#import "PlatformLib.h"
#import "HttpEngineIOS.h"

// 实现请求代理
@interface HttpEngineDelegate : NSObject<NSURLSessionDataDelegate>

@property (strong) id<HttpCallback> callback;
@property (strong) NSString* url;

@end

@interface HttpEngineIOS()

@property (nonatomic, strong) NSString* postData;

@end

@implementation HttpEngineIOS

-(void) loadUrl: (HttpMethod) method url:(NSString*)url headers:(NSDictionary*)headers callback:(id<HttpCallback>)cb {
    //self.receiveData = [NSMutableData new];
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
    
    if (headers) {
        NSLog(@"HttpHeaders: %@",headers);
        [request setAllHTTPHeaderFields:headers];
    }
    
    // 使用默认配置
    NSURLSessionConfiguration* config = [NSURLSessionConfiguration defaultSessionConfiguration];
    
    // 创建操作队列
    NSOperationQueue* queue = [NSOperationQueue new];
    
    HttpEngineDelegate* delegate = [HttpEngineDelegate new];
    delegate.callback = cb;
    delegate.url = url;
    
    // 创建Session, 设置代理
    NSURLSession* session = [NSURLSession sessionWithConfiguration:config delegate:delegate delegateQueue:queue];
    
    // 创建任务对象
    NSURLSessionTask* task = [session dataTaskWithRequest:request];
    
    //[task setValue:cb forKey:@"haha"];
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

-(void)loadUrlWithData: (HttpMethod) method url:(const char *)url headers:(NSDictionary*)headers callback:(id<HttpCallback>)cb{
    NSString* nsUrl = [[NSString alloc] initWithUTF8String: url];
    [self loadUrl:method url:nsUrl headers:headers callback:cb];
}

-(void)setData: (const char*)data size:(NSInteger)size {
    //self.postData = [[NSString alloc] initWithUTF8String: data];
    self.postData = [[NSString alloc]initWithBytes:data length:size encoding:NSUTF8StringEncoding];
}

@end

@implementation HttpEngineDelegate

#pragma mark --NSURLSessionDownloadDelegate

-(void)URLSession:(NSURLSession*)session task:(NSURLSessionTask*)task didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential* _Nullable))completionHandler
{
    NSLog(@"URLSession challenge:%@",challenge.protectionSpace);
    
    if (![challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust]) {
        return;
    }
    /*
     NSURLSessionAuthChallengeUseCredential 使用证书
     NSURLSessionAuthChallengePerformDefaultHandling  忽略证书 默认的做法
     NSURLSessionAuthChallengeCancelAuthenticationChallenge 取消请求,忽略证书
     NSURLSessionAuthChallengeRejectProtectionSpace 拒绝,忽略证书
     */
    
    // 信任自签名证书
    NSURLCredential* credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
    
    completionHandler(NSURLSessionAuthChallengeUseCredential,credential);
}

// 接受网络数据
- (void)URLSession:(NSURLSession*)session dataTask:(NSURLSessionDataTask*)dataTask didReceiveData:(NSData*)data {
    if (self.callback != nil) {
        [self.callback onDataReceive:data];
    }
}

- (void)URLSession:(NSURLSession*)session task:(NSURLSessionTask*)task
didCompleteWithError:(nullable NSError *)error {
    //NSString* json = [[NSString alloc] initWithData:self.receiveData encoding:(NSUTF8StringEncoding)];
    //NSLog(@"Result New data = %@",json);
    NSLog(@"HttpEngine finished url: %@", self.url);
    if (!self.callback) {
        return;
    }
    
    if (error) {
        NSLog(@"HttpEngine finished error: %@", [error localizedDescription]);
        [self.callback onLoadError];
        return;
    }
    

    [self.callback onLoadFinished];
}


@end

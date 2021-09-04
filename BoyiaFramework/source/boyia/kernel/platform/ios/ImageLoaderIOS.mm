//
//  ImageLoader.m
//  core
//
//  Created by yanbo on 2021/8/21.
//

#import <Foundation/Foundation.h>

#import "ImageLoaderIOS.h"
#import "HttpEngineIOS.h"

#include "UIThreadClientMap.h"
#include "UIThread.h"
#include "ImageIOS.h"

@interface ImageLoaderIOS()

@property (nonatomic, strong) HttpEngineIOS* engine;

@end

@interface ImageClient : NSObject<HttpCallback>

@property (strong) NSMutableData* receiveData;
@property (nonatomic, assign) int clientId;

@end

// 实现ImageClient
@implementation ImageClient

-(instancetype)initWithClientId:(int)cid {
    self = [super init];
    if (self != nil) {
        self.receiveData = [NSMutableData new];
        self.clientId = cid;
    }
    
    return self;
}

-(void)onDataReceive:(NSData*)data {
    [self.receiveData appendData:data];
}

-(void)onLoadFinished {
    //NSLog(@"Result New data = %@",json);
    yanbo::UIThread::instance()->postClosureTask([self] {
        //NSString* json = [[NSString alloc] initWithData:self.receiveData encoding:(NSUTF8StringEncoding)];
        OwnerPtr<String> data = new String((const LUint8*)self.receiveData.bytes, (int)self.receiveData.length);
        util::ImageIOS* image = static_cast<util::ImageIOS*>(yanbo::UIThreadClientMap::instance()->findUIThreadClient(self.clientId));
        if (image) {
            image->setData(data);
            image->onClientCallback();
        }
    });
}

@end

// 实现ImageLoader
@implementation ImageLoaderIOS

// 单例模式
static ImageLoaderIOS* _instance = nil;
+(instancetype)shareInstance {
    static dispatch_once_t shareOnce;
    dispatch_once(&shareOnce, ^{
        _instance = [[super allocWithZone:NULL] init];
    });
    
    return _instance;
}

// 禁止重新分配内存
+(id) allocWithZone:(struct _NSZone *)zone {
    return [ImageLoaderIOS shareInstance] ;
}

// 禁止拷贝
-(id) copyWithZone:(struct _NSZone *)zone {
    return [ImageLoaderIOS shareInstance] ;
}

- (instancetype)init {
    if (self = [super init]) {
        // new表示执行alloc以及init
        self.engine = [HttpEngineIOS new];
    }
    
    return self;
}

// 开始加载图片
-(void)loadImage:(NSString *)url clientId:(int)cid {
    ImageClient* client = [[ImageClient alloc]initWithClientId:cid];
    [self.engine loadUrl:kHttpGet url:url callback:client];
}

@end

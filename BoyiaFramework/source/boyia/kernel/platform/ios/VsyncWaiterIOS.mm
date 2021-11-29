#ifndef VsyncWaiterIOS_h
#define VsyncWaiterIOS_h

#include "VsyncWaiter.h"
#include "UIThread.h"

#include <Foundation/Foundation.h>
#include <QuartzCore/CADisplayLink.h>
#include <UIKit/UIKit.h>

@interface VsyncClient : NSObject
-(instancetype)initVsync:(yanbo::VsyncWaiter::Callback)callback;
-(void)await;
-(void)removeVsync;
@end

namespace yanbo {
// TODO
class VsyncWaiterIOS : public VsyncWaiter {
public:
    VsyncWaiterIOS()
    {
        auto callback = [this]() {
            fireCallback();
        };
        m_client = [[VsyncClient alloc] initVsync:callback];
    }
    
    ~VsyncWaiterIOS()
    {
        [m_client removeVsync];
    }
    
    // 唤醒vsync信号
    virtual LVoid awaitVSync()
    {
        [m_client await];
    }
private:
    VsyncClient* m_client;
};

VsyncWaiter* VsyncWaiter::createVsyncWaiter()
{
    return new VsyncWaiterIOS();
}
}

@implementation VsyncClient {
    yanbo::VsyncWaiter::Callback _callback;
    CADisplayLink* _displayLink;
}

-(instancetype)initVsync:(yanbo::VsyncWaiter::Callback)callback {
    self = [super init];
    if (self) {
        _callback = std::move(callback);
        _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplayLink:)];
        _displayLink.paused = YES;
        
//        yanbo::UIThread::instance()->postClosureTask([client = self]() {
//            [client->_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
//        });
        // 必须加入带有NSRunLoop的线程
        dispatch_async(dispatch_get_main_queue(), ^{
            [self->_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        });
    }
    
    return self;
}

-(void)await {
    _displayLink.paused = NO;
}

-(void)onDisplayLink:(CADisplayLink*)link {
    // TODO do something render
    _callback();
    _displayLink.paused = YES;
}

// 从runloop中移除
-(void)removeVsync {
    [_displayLink invalidate];
}
@end
#endif

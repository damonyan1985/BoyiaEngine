#ifndef VsyncWaiterIOS_h
#define VsyncWaiterIOS_h

#include "VsyncWaiter.h"

#include <Foundation/Foundation.h>
#include <QuartzCore/CADisplayLink.h>
#include <UIKit/UIKit.h>

@interface VsyncClient : NSObject
-(instancetype)initVsync;
-(void)await;
@end

namespace yanbo {
// TODO
class VsyncWaiterIOS : public VsyncWaiter {
public:
    VsyncWaiterIOS()
    {
        m_client = [[VsyncClient alloc] initVsync];
    }
    
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
    CADisplayLink* displayLink;
}

-(instancetype)initVsync {
    self = [super init];
    if (self) {
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onDisplayLink:)];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    }
    
    return self;
}

-(void)await {
    displayLink.paused = NO;
}

- (void)onDisplayLink:(CADisplayLink*)link {
    // TODO do something
    displayLink.paused = YES;
}
@end
#endif

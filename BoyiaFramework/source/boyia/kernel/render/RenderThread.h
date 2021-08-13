#ifndef RenderThread_h
#define RenderThread_h

#include "MessageThread.h"

namespace yanbo {
// 渲染与UI逻辑进行分离
class RenderThread : public MessageThread {
public:
    virtual LVoid handleMessage(Message* msg);

private:
    
};
}

#endif

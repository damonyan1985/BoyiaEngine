#ifndef RenderThread_h
#define RenderThread_h

#include "BoyiaPtr.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "KVector.h"
#include "MessageThread.h"
#include "RenderCommand.h"

namespace yanbo {
// 渲染与UI逻辑进行分离
using RenderCommands = KVector<BoyiaPtr<RenderCommand>>;
// key对应HTMLView的id
using RenderCommandMap = HashMap<HashInt, RenderCommands>;

class RenderMessage : public BoyiaRef {
public:
    RenderMessage() = default;
    // HtmlView渲染的顺序
    KVector<LInt> idOrders;
    // 通过Id查找对应的RenderCommands
    RenderCommandMap commandMap;
};
// Buffer是一个hashmap，其中key为boyia app的唯一ID
using RenderCommandBuffer = HashMap<HashInt, BoyiaPtr<RenderMessage>>;
class RenderThread : public MessageThread {
public:
    virtual LVoid handleMessage(Message* msg);

private:
    RenderCommandBuffer m_buffer;
};
}

#endif

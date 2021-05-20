#ifndef MessageQueue_h
#define MessageQueue_h

#include "AutoLock.h"
#include "KList.h"

namespace yanbo {

class Message {
public:
    Message();
    LVoid msgRecycle();

public:
    LInt type;
    LVoid* obj;
    LIntPtr arg0;
    LIntPtr arg1;
    long when;
    LBool recycle;
    LBool inCache;
};

class MessageCache {
public:
    MessageCache();
    virtual Message* obtain();
    LVoid initCache();

private:
    Message* m_cache;
};

typedef KList<Message*> MessageList;
class MessageQueue : public MessageCache {
public:
    Message* obtain();
    LVoid push(Message* msg);
    LVoid removeMessage(LInt type);
    LBool hasMessage(LInt type);
    LInt size();
    Message* poll();

private:
    MessageList m_list;
    Lock m_queueLock;
};
}
#endif

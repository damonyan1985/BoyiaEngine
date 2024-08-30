#ifndef MessageQueue_h
#define MessageQueue_h

#include "AutoLock.h"
#include "KList.h"

namespace yanbo {

class Message {
public:
    Message();

public:
    LInt type;
    LVoid* obj;
    LIntPtr arg0;
    LIntPtr arg1;
    long when;
    LBool inCache;
    Message* next;
};

class MessageCache {
public:
    MessageCache();
    virtual Message* obtain();
    virtual LVoid freeMessage(Message* msg);

private:
    LVoid initCache();

    Message* m_cache;
    Message* m_freeList;
    LInt m_useIndex;
};

typedef KList<Message*> MessageList;
class MessageQueue : public MessageCache {
public:
    Message* obtain();
    LVoid freeMessage(Message* msg);

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

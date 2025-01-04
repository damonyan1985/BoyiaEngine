/*
 * Boyia异步事件回调处理
 */
#ifndef BoyiaAsyncEvent_h
#define BoyiaAsyncEvent_h

#include "BoyiaCore.h"
#include "KList.h"
#include "OwnerPtr.h"
#include "BoyiaRuntime.h"

namespace boyia {

class AsyncEvent {
public:
    virtual ~AsyncEvent();

protected:
    virtual LVoid run() = 0;

private:
    LVoid execute();
};

class BoyiaAsyncMapTable;
class BoyiaAsyncEvent : public AsyncEvent {
public:
    static LVoid callbackString(String& result, BoyiaValue* callback, BoyiaRuntime* runtime);
    
    BoyiaAsyncEvent(BoyiaValue* obj, BoyiaRuntime* runtime);

    virtual ~BoyiaAsyncEvent();

    virtual LVoid run();

    virtual LVoid callback() = 0;

protected:
    BoyiaValue m_obj;
    LInt m_uniqueId;
    BoyiaRuntime* m_runtime;
    friend class BoyiaAsyncMapTable;
};

class BoyiaAsyncEventManager {
public:
    BoyiaAsyncEventManager();
    ~BoyiaAsyncEventManager();
    // Create async event and add to table
    LVoid registerEvent(BoyiaAsyncEvent* obj);

    // Judge the object is exist
    LBool hasObject(BoyiaValue* obj, LInt uniqueId);

    // When remove the object, needs to remove all event
    LVoid removeAllEvent(LIntPtr ptr);

    // Remove Aysnc Event From Map
    LVoid removeEvent(BoyiaAsyncEvent* event);

    LInt increment();

private:
    BoyiaAsyncMapTable* m_table;
    LInt m_uniqueId;
};

}

#endif

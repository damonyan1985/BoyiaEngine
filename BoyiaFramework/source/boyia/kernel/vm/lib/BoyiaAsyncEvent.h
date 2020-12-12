/*
 * Boyia异步事件回调处理
 */
#ifndef BoyiaAsyncEvent_h
#define BoyiaAsyncEvent_h

#include "BoyiaCore.h"
#include "KList.h"
#include "UIThread.h"
#include "OwnerPtr.h"

namespace boyia {
class BoyiaAsyncEvent;
class BoyiaAsyncMapTable;
class BoyiaAsyncEvent : public yanbo::UIEvent {
public:
    // Create async event and add to table
    static LVoid registerEvent(BoyiaAsyncEvent* obj);

    // Judge the object is exist
    static LBool hasObject(BoyiaValue* obj);

    // When remove the object, needs to remove all event
    static LVoid removeAllEvent(LIntPtr ptr);

    // Remove Aysnc Event From Map
    static LVoid removeEvent(BoyiaAsyncEvent* event);

    BoyiaAsyncEvent(BoyiaValue* obj);

    virtual ~BoyiaAsyncEvent();

    virtual LVoid run();

    virtual LVoid callback() = 0;

private:
    static BoyiaAsyncMapTable s_table;

protected:
    BoyiaValue m_obj;
    friend class BoyiaAsyncMapTable;
};

}

#endif

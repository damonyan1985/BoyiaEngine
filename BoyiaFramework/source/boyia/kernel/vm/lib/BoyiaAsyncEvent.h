/*
 * Boyia异步事件回调处理
 */
#ifndef BoyiaAsyncEvent_h
#define BoyiaAsyncEvent_h

#include "BoyiaCore.h"
#include "KList.h"
#include "UIThread.h"

namespace boyia {
class AsyncObject;
using BoyiaAsyncMapTable = KList<AsyncObject*>;

class BoyiaAsyncEvent : public yanbo::UIEvent {
public:
    // Create async event and add to table
    static LVoid registerObject(BoyiaValue* obj);

    // Judge the object is exist
    static LBool hasObject(BoyiaValue* obj);

    // When remove the object, needs to remove all callback
    static LVoid removeObject(LIntPtr ptr);

    BoyiaAsyncEvent(BoyiaValue* obj);

    virtual LVoid run();

    virtual LVoid callback() = 0;

private:
    static BoyiaAsyncMapTable s_table;

protected:
    BoyiaValue m_obj;
};

}

#endif

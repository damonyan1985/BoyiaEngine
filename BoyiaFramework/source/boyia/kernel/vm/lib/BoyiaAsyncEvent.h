/*
 * Boyia异步事件回调处理
 */
#ifndef BoyiaAsyncEvent_h
#define BoyiaAsyncEvent_h

#include "BoyiaCore.h"
#include "KList.h"

namespace boyia {
class BoyiaAsyncMap;

using BoyiaAsyncMapTable = KList<BoyiaAsyncMap*>;
class BoyiaAsyncEvent {
public:
    // Create async event and add to table
    BoyiaAsyncEvent(BoyiaValue* callback, BoyiaValue* obj);

    // When remove the object, needs to remove all callback
    static LVoid removeObject(LIntPtr ptr);

private:
    static BoyiaAsyncMapTable s_table;
};

}

#endif

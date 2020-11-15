#include "BoyiaAsyncEvent.h"
#include "KVector.h"

namespace boyia {
class AsyncObject {
public:
    AsyncObject()
        : object(kBoyiaNull)
    {
    }
    // Save object address
    LIntPtr object;
};

BoyiaAsyncMapTable BoyiaAsyncEvent::s_table;

LVoid BoyiaAsyncEvent::registerObject(BoyiaValue* obj)
{
    BoyiaAsyncMapTable::Iterator iter = s_table.begin();
    BoyiaAsyncMapTable::Iterator end = s_table.end();
    for (; iter != end; ++iter) {
        if (obj->mValue.mObj.mPtr == (*iter)->object) {
            return;
        }
    }

    // the address alloc in heap
    AsyncObject* object = new AsyncObject();
    object->object = obj->mValue.mObj.mPtr;
    s_table.push(object);
}

LBool BoyiaAsyncEvent::hasObject(BoyiaValue* obj)
{
    BoyiaAsyncMapTable::Iterator iter = s_table.begin();
    BoyiaAsyncMapTable::Iterator end = s_table.end();
    for (; iter != end; ++iter) {
        if (obj->mValue.mObj.mPtr == (*iter)->object) {
            return LTrue;
        }
    }

    return LFalse;
}

LVoid BoyiaAsyncEvent::removeObject(LIntPtr ptr)
{
    BoyiaAsyncMapTable::Iterator iter = s_table.begin();
    BoyiaAsyncMapTable::Iterator end = s_table.end();
    for (; iter != end; ++iter) {
        if (ptr == (*iter)->object) {
            s_table.erase(iter);
            return;
        }
    }
}

BoyiaAsyncEvent::BoyiaAsyncEvent(BoyiaValue* obj)
{
    ValueCopy(&m_obj, obj);
    BoyiaAsyncEvent::registerObject(obj);
}

LVoid BoyiaAsyncEvent::run()
{
    if (!BoyiaAsyncEvent::hasObject(&m_obj)) {
        return;
    }

    callback();
}
}
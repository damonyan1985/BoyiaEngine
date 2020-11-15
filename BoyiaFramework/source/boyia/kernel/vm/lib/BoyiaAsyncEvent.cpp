#include "BoyiaAsyncEvent.h"
#include "KVector.h"

namespace boyia {
class BoyiaAsyncMap {
public:
    BoyiaAsyncMap()
        : object(kBoyiaNull)
        , callbacks(0, 5)
    {
    }
    // Save object address
    LIntPtr object;
    // Save callback address
    KVector<BoyiaValue*> callbacks;
};

KList<BoyiaAsyncMap*> BoyiaAsyncEvent::s_table;
BoyiaAsyncEvent::BoyiaAsyncEvent(BoyiaValue* callback, BoyiaValue* obj)
{
    // the address alloc in heap
    LIntPtr objPtr = obj->mValue.mObj.mPtr;

    BoyiaAsyncMapTable::Iterator iter = s_table.begin();
    BoyiaAsyncMapTable::Iterator end = s_table.end();

    for (; iter != end; iter++) {
        if (objPtr == (*iter)->object) {
            (*iter)->callbacks.addElement(callback);
            return;
        }
    }

    BoyiaAsyncMap* map = new BoyiaAsyncMap();
    map->object = objPtr;
    map->callbacks.addElement(callback);
    s_table.push(map);
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
}
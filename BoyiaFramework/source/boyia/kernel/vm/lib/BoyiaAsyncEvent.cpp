#include "BoyiaAsyncEvent.h"

namespace boyia {
class BoyiaAsyncMap {
public:
    BoyiaAsyncMap()
        : object(kBoyiaNull)
    {
    }
    // Save object address
    LIntPtr object;
    // Save callback address
    KVector<BoyiaValue*> callbacks;
};

KVector<BoyiaAsyncMap*> BoyiaAsyncEvent::s_table;
BoyiaAsyncEvent::BoyiaAsyncEvent(BoyiaValue* callback, BoyiaValue* obj)
{
    // 堆上分配的对象的地址
    LIntPtr objPtr = obj->mValue.mObj.mPtr;
    for (LInt i = 0; i < s_table.size(); i++) {
        if (objPtr == s_table[i]->object) {
            s_table[i]->callbacks.addElement(callback);
            return;
        }
    }

    BoyiaAsyncMap* map = new BoyiaAsyncMap();
    map->object = objPtr;
    map->callbacks.addElement(callback);
    s_table.addElement(map);
}
}
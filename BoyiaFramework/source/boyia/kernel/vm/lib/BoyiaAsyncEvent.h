/*
 * Boyia�첽�ص��¼�����
 */
#ifndef BoyiaAsyncEvent_h
#define BoyiaAsyncEvent_h

#include "BoyiaCore.h"
#include "KVector.h"

namespace boyia {
class BoyiaAsyncMap;
class BoyiaAsyncEvent {
public:
    // Create async event and add to table
    BoyiaAsyncEvent(BoyiaValue* callback, BoyiaValue* obj);

private:
    static KVector<BoyiaAsyncMap*> s_table;
};

}

#endif



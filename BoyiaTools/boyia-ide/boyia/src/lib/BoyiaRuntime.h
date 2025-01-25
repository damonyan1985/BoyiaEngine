#ifndef BoyiaRuntime_h
#define BoyiaRuntime_h

#include "IDCreator.h"
#include "UtilString.h"
#include "BoyiaLib.h"
#include "HashMap.h"
#include "HashUtil.h"

namespace boyia {

class BoyiaAsyncEventManager;
class BoyiaRuntime {
public:
    BoyiaRuntime();
    ~BoyiaRuntime();

    LVoid init();
    LVoid compile(const String& script);
    LVoid compileFile(const String& path);
    LVoid* vm() const;
    util::IDCreator* idCreator() const;
    LInt findNativeFunc(LUintPtr key) const;
    LInt callNativeFunction(LInt idx) const;
    LVoid* memoryPool() const;
    LVoid* garbageCollect() const;
    LBool needCollect() const;
    LVoid prepareDelete(LVoid* ptr);
    LVoid setGcRuning(LBool isRuning);
    LVoid collectGarbage();
    BoyiaAsyncEventManager* eventManager() const;
    LVoid* createMemoryBackup();
    LVoid changeMemoryPool(LVoid* pool);
    // 调用平台相关API
    LVoid callPlatformApi(const String& params, BoyiaValue* propCB);
    LVoid consumeMicroTask();

private:
    LVoid initNativeFunction();
    LVoid appendNative(LUintPtr id, NativePtr ptr);

    LVoid* m_memoryPool;
    OwnerPtr<util::IDCreator> m_idCreator;
    KVector<NativeFunction> m_nativeFunTable;
    // Last init vm
    LVoid* m_vm;
    LVoid* m_gc;
    LInt m_nativeSize;
    LBool m_isGcRuning;
    OwnerPtr<BoyiaAsyncEventManager> m_eventManager;
    HashMap<HashString, LBool> m_programSet;
};
}

#endif
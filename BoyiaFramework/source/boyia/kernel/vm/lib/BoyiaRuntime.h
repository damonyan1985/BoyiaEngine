#ifndef BoyiaRuntime_h
#define BoyiaRuntime_h

#include "BoyiaLib.h"
#include "IDCreator.h"
#include "UIView.h"
#include "UtilString.h"
#include "DOMBuilder.h"
#include "BoyiaDebugger.h"

namespace yanbo {
class Application;
}

namespace boyia {

using BoyiaDomMap = HashMap<HashString, yanbo::DOMBuilder*>;
class BoyiaAsyncEventManager;
class BoyiaRuntime {
public:
    BoyiaRuntime(yanbo::Application* app);
    ~BoyiaRuntime();

    LVoid init();
    LVoid compile(const String& script);
    LVoid* vm() const;
    yanbo::UIView* view() const;
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
    BoyiaDomMap* domMap() const;
    LVoid* createMemoryBackup();
    LVoid changeMemoryPool(LVoid* pool);
    // 调用平台相关API
    LVoid callPlatformApi(const String& params, BoyiaValue* propCB);
    LVoid consumeMicroTask();
    LBool isLoadExeFile() const;
    LVoid runExeFile();
    BoyiaDebugger* debugger() const;

private:
    LVoid initNativeFunction();
    LVoid appendNative(LUintPtr id, NativePtr ptr);

    yanbo::Application* m_app;
    LVoid* m_memoryPool;
    OwnerPtr<util::IDCreator> m_idCreator;
    KVector<NativeFunction> m_nativeFunTable;
    // Last init vm
    LVoid* m_vm;
    LVoid* m_gc;
    LInt m_nativeSize;
    LBool m_isGcRuning;
    LBool m_isLoadExeFile;
    OwnerPtr<BoyiaDomMap> m_domMap;
    OwnerPtr<BoyiaAsyncEventManager> m_eventManager;
    OwnerPtr<BoyiaDebugger> m_debugger;
};
}

#endif

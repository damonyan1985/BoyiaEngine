#ifndef BoyiaRuntime_h
#define BoyiaRuntime_h

#include "BoyiaLib.h"
#include "IDCreator.h"
#include "UIView.h"
#include "UtilString.h"

namespace yanbo {
class Application;
}

namespace boyia {
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

private:
    LVoid initNativeFunction();
    LVoid appendNative(LUintPtr id, NativePtr ptr);

    yanbo::Application* m_app;
    LVoid* m_memoryPool;
    util::IDCreator* m_idCreator;
    NativeFunction* m_nativeFunTable;
    // Last init vm
    LVoid* m_vm;
    LVoid* m_gc;
    LInt m_nativeSize;
    LBool m_isGcRuning;
    BoyiaAsyncEventManager* m_eventManager;
};
}

#endif
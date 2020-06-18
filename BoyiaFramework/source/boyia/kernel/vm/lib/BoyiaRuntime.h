#ifndef BoyiaRuntime_h
#define BoyiaRuntime_h

#include "UtilString.h"
#include "UIView.h"
#include "IDCreator.h"
#include "BoyiaLib.h"

namespace yanbo {
class Application;
}

namespace boyia {
class BoyiaRuntime {
public:
    BoyiaRuntime(yanbo::Application* app);
    ~BoyiaRuntime();

    LVoid compile(const String& script);
    LVoid* vm() const;
    yanbo::UIView* view() const;
    util::IDCreator* idCreator() const;
    LInt findNativeFunc(LUintPtr key) const;
    LInt callNativeFunction(LInt idx) const;

private:
    LVoid initNativeFunction();
    LVoid appendNative(LUintPtr id, NativePtr ptr);

    LVoid* m_vm;
    yanbo::Application* m_app;
    util::IDCreator* m_idCreator;
    NativeFunction* m_nativeFunTable;
    LInt m_nativeSize;
};
}

#endif
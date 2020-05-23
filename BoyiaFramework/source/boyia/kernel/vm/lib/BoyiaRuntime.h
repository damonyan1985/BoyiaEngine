#ifndef BoyiaRuntime_h
#define BoyiaRuntime_h

#include "UtilString.h"
#include "UIView.h"

namespace yanbo {
class Application;
}

namespace boyia {
class BoyiaRuntime {
public:
    BoyiaRuntime(yanbo::Application* app);
    ~BoyiaRuntime();

    LVoid compile(const String& script);
    LVoid useVM();
    LVoid* vm() const;
    yanbo::UIView* view() const;

private:
    LVoid* m_vm;
    yanbo::Application* m_app;
};
}

#endif
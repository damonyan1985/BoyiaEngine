#ifndef Application_h
#define Application_h

#include "AppInfo.h"
#include "BoyiaRuntime.h"
#include "UIView.h"

namespace yanbo {
class Application {
public:
    Application(AppInfo* info);
    virtual ~Application();

    UIView* view() const;
    boyia::BoyiaRuntime* runtime() const;
    LVoid init(const String& entryPage);
    const AppInfo* appInfo() const;

private:
    OwnerPtr<UIView> m_view;
    OwnerPtr<boyia::BoyiaRuntime> m_runtime;
    AppInfo* m_info;
};
}

#endif
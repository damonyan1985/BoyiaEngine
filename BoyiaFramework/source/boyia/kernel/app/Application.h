#ifndef Application_h
#define Application_h

#include "AppInfo.h"
#include "BoyiaRuntime.h"
#include "UIView.h"

namespace yanbo {
class Application {
public:
    Application(AppInfo* info);
    ~Application();

    UIView* view() const;
    boyia::BoyiaRuntime* runtime() const;
    LVoid init(const String& entryPage);

private:
    UIView* m_view;
    boyia::BoyiaRuntime* m_runtime;
    AppInfo* m_info;
};
}

#endif
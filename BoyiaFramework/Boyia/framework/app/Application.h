#ifndef Application_h
#define Application_h

#include "AppInfo.h"
#include "AppVM.h"
#include "UIView.h"

namespace yanbo {
class Application {
public:
    Application(AppInfo* info);
    ~Application();

    UIView* view() const;
    AppVM* vm() const;
    LVoid init(const String& entryPage);

private:
    UIView* m_view;
    AppVM* m_vm;
    AppInfo* m_info;
};
}

#endif
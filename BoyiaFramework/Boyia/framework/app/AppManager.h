#ifndef AppManager_h
#define AppManager_h

#include "AppLoader.h"
#include "AppStack.h"
#include "AppThread.h"
#include "LEvent.h"
#include "LGraphic.h"
#include "UIThread.h"

namespace yanbo {
class AppManager {
public:
    static AppManager* instance();
    LVoid start();
    LVoid setViewport(const LRect& rect);
    const LRect& getViewport() const;
    Application* currentApp();
    UIThread* uiThread() const;
    AppThread* appThread() const;
    LVoid launchApp(AppInfo* info);

    LVoid handleTouchEvent(LInt type, LInt x, LInt y);

private:
    AppManager();
    ~AppManager();

    AppStack m_stack;
    LRect m_clientRect;
    AppLoader* m_loader;
    AppThread* m_appThread;
    UIThread* m_uiThread;
};
}

#endif
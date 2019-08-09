#ifndef AppManager_h
#define AppManager_h

#include "AppLoader.h"
#include "AppStack.h"
#include "BoyiaThread.h"
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
    BoyiaThread* appThread() const;

private:
    AppManager();
    ~AppManager();

    AppStack m_stack;
    LRect m_clientRect;
    AppLoader* m_loader;
    BoyiaThread* m_appThread;
    UIThread* m_uiThread;
};
}

#endif
#ifndef AppManager_h
#define AppManager_h

#include "AppLoader.h"
#include "AppStack.h"
#include "LEvent.h"
#include "LGraphic.h"
#include "NetworkBase.h"
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
    LVoid launchApp(AppInfo* info);
    // 调用其他app功能，TODO 权限控制
    LVoid pushApp(AppInfo* info);
    NetworkBase* network() const;
    LVoid handleTouchEvent(LInt type, LInt x, LInt y);

private:
    AppManager();
    ~AppManager();

    AppStack m_stack;
    LRect m_clientRect;
    AppLoader* m_loader;
    UIThread* m_uiThread;
    // network只能在UI线程中使用
    // 其他线程中使用需要自己创建loader
    NetworkBase* m_network;
};
}

#endif

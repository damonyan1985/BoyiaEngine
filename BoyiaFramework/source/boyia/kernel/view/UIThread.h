#ifndef UIThread_h
#define UIThread_h

#include "BoyiaPtr.h"
#include "LEvent.h"
#include "LGdi.h"
#include "MessageThread.h"
#include <functional>

namespace yanbo {
class AppManager;
class VsyncWaiter;

class UIEvent {
public:
    virtual ~UIEvent();

protected:
    virtual LVoid run() = 0;

private:
    LVoid execute();
    friend class UIThread;
};

using UIClosure = std::function<void()>;
class UIThread LFinal : public MessageThread {
public:
    enum MessageType {
        kUiInit = 1,
        kUiReset,
        kUiDraw,
        kUiDrawOnly,
        kUiCreateTexture,
        kUiSubmit,
        kUiTouchEvent,
        kUiKeyEvent,
        kUiSetInput,
        kUiVideoUpdate,
        kUiClientCallback,
        kUiOnKeyboardShow,
        kUiOnKeyboardHide,
        kUiOperationExec,
        kUiDestory,
        kUiClosureTask,
        kUiEvent,
        kUiInitApp,
        kVsyncDraw,
        kPlatformViewUpdate,
        kFlingEvent,
    };
    static UIThread* instance();

    UIThread(AppManager* manager);
    virtual ~UIThread();

    LVoid initContext();
    LVoid resetContext();

    LVoid draw(LVoid* item);
    LVoid drawOnly(LVoid* item);
    LVoid submit();
    // EditText渲染
    LVoid setInputText(const String& text, LIntPtr item);
    // Video渲染
    LVoid videoUpdate(LIntPtr item);
    LVoid clientCallback(LIntPtr item);
    // ui线程运行lambda表达式
    LVoid postClosureTask(const UIClosure& func);
    // 回调处理
    LVoid sendUIEvent(UIEvent* event);
    LVoid destroy();
    LVoid handleTouchEvent(LTouchEvent* evt);
    LVoid handleKeyEvent(LKeyEvent* evt);
    LVoid uiExecute();
    LVoid onKeyboardShow(LIntPtr item, LInt keyboardHeight);
    LVoid onKeyboardHide(LIntPtr item, LInt keyboardHeight);

    LGraphicsContext* graphics() const;
    LVoid initApp(const String& entry);
    virtual LVoid handleMessage(Message* msg);
    LVoid drawUI(LVoid* view);
    LVoid vsyncDraw();

    LVoid platformViewUpdate(const String& id);
    LVoid handleFlingEvent(LFlingEvent* evt);
    
private:
    LVoid reset(Message* msg);
    LVoid flush();

    LGraphicsContext* m_gc;
    AppManager* m_manager;
    BoyiaPtr<VsyncWaiter> m_vsyncWaiter;
};
}
#endif

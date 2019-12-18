#ifndef UIThread_h
#define UIThread_h

//#include "GLContext.h"
#include "LEvent.h"
#include "LGdi.h"
#include "MessageThread.h"

namespace yanbo {
class AppManager;

class UIEvent {
public:
    virtual ~UIEvent();

protected:
    virtual LVoid run() = 0;

private:
    LVoid execute();
    friend class UIThread;
};

class UIThread : public MessageThread {
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
        kUiRunAnimation,
        kUiEvent,
        kUiInitApp
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
    // Anim渲染
    LVoid runAnimation(LVoid* callback);
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

private:
    LVoid resetGL(Message* msg);
    LVoid flush();

    LGraphicsContext* m_gc;
    AppManager* m_manager;
};
}
#endif

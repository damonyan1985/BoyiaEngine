#ifndef PaintThread_h
#define PaintThread_h

#include "GLContext.h"
#include "LEvent.h"
#include "LGdi.h"
#include "MessageThread.h"

namespace yanbo {
class AppManager;

class UIEvent {
public:
    LVoid ~UIEvent();
    virtual LVoid run() = 0;
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
        kUiImageLoaded,
        kUiOnKeyboardShow,
        kUiOnKeyboardHide,
        kUiOperationExec,
        kUiDestory,
        kUiRunAnimation,
        kUiEvent
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
    LVoid imageLoaded(LIntPtr item);
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
    virtual LVoid handleMessage(Message* msg);

private:
    LVoid resetGL(Message* msg);
    LVoid flush();

    LVoid drawUI(LVoid* view);
    LGraphicsContext* m_gc;
    AppManager* m_manager;
};
}
#endif

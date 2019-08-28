#ifndef PaintThread_h
#define PaintThread_h

#include "GLContext.h"
#include "LEvent.h"
#include "LGdi.h"
#include "MiniMessageThread.h"

namespace yanbo {
class AppManager;
class UIThread : public MiniMessageThread {
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
        kUiRunAnimation
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
    LVoid destroy();
    LVoid handleTouchEvent(LTouchEvent* evt);
    LVoid handleKeyEvent(LKeyEvent* evt);
    LVoid uiExecute();
    LVoid onKeyboardShow(LIntPtr item, LInt keyboardHeight);
    LVoid onKeyboardHide(LIntPtr item, LInt keyboardHeight);

    LGraphicsContext* graphics() const;
    virtual LVoid handleMessage(MiniMessage* msg);

private:
    LVoid resetGL(MiniMessage* msg);
    LVoid flush();

    LVoid drawUI(LVoid* view);
    LGraphicsContext* m_gc;
    AppManager* m_manager;
};
}
#endif

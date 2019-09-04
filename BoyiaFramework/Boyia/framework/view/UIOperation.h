#ifndef UIOperation_h
#define UIOperation_h

#include "KVector.h"
#include "MessageQueue.h"
#include "UtilString.h"

namespace yanbo {
// UI线程一切操作【现有】Widget DOM必须走UIOperation
// 最后将由PaintThread来执行UIOperation::execute()
// 来完成绘制工作
class UIOperation : public MessageCache {
public:
    enum UIOpType {
        UIOP_ADDCHILD,
        UIOP_SETINPUT,
        UIOP_SETIMAGE_URL,
        UIOP_LOADIMAGE_URL,
        UIOP_SETTEXT,
        UIOP_DRAW,
        UIOP_APPLY_DOM_STYLE,
    };

    static UIOperation* instance();

    UIOperation();
    ~UIOperation();

    LVoid execute();
    virtual Message* obtain();
    LVoid swapBuffer();

    // operation
    LVoid opSetText(LVoid* view, const String& text);
    LVoid opAddChild(LVoid* view, LVoid* child);
    LVoid opSetImageUrl(LVoid* view, const String& url);
    LVoid opLoadImageUrl(LVoid* view, const String& url);
    LVoid opViewDraw(LVoid* view);
    LVoid opApplyDomStyle(LVoid* view);

private:
    LVoid viewAddChild(Message* msg);
    LVoid viewSetText(Message* msg);
    LVoid viewDraw(Message* msg);

    LVoid swapBufferImpl();

    KVector<Message*>* m_msgs;
    KVector<Message*>* m_swapMsgs;
    Mutex m_uiMutex;
};
}
#endif

#ifndef UIOperation_h
#define UIOperation_h

#include "KVector.h"
#include "MessageQueue.h"
#include "UtilString.h"
#include "HtmlView.h"
#include "WeakPtr.h"

namespace yanbo {
// UI线程一切操作【现有】Widget DOM必须走UIOperation
// 最后将由PaintThread来执行UIOperation::execute()
// 来完成绘制工作
//class HtmlView;
class UIOperation LFinal: public MessageCache {
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

    UIOperation();
    ~UIOperation();

    LVoid execute();
    LInt createView();
    
    virtual Message* obtain();
    LVoid swapBuffer();

    // operation
    LVoid opSetText(HtmlView* view, const String& text);
    LVoid opAddChild(HtmlView* view, HtmlView* child);
    LVoid opSetImageUrl(HtmlView* view, const String& url);
    LVoid opLoadImageUrl(HtmlView* view, const String& url);
    LVoid opViewDraw(HtmlView* view);
    LVoid opApplyDomStyle(HtmlView* view);
    LVoid opSetInput(HtmlView* view, const String& text);

private:
    LVoid viewAddChild(Message* msg);
    LVoid viewSetText(Message* msg);
    LVoid viewDraw(Message* msg);
    LVoid viewSetInput(Message* msg);

    KVector<Message*>* m_msgs;
    KVector<WeakPtr<HtmlView>>* m_views;
};
}
#endif

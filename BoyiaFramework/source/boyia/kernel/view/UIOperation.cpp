#include "UIOperation.h"
#include "HtmlView.h"
#include "ImageView.h"
#include "InputView.h"
#include "TextView.h"
#include "UIThread.h"
#include "UIView.h"

namespace yanbo {
#define MAX_OPMSG_SIZE 50

UIOperation::UIOperation()
    : m_msgs(new KVector<Message*>(0, MAX_OPMSG_SIZE))
//, m_swapMsgs(new KVector<Message*>(0, MAX_OPMSG_SIZE))
{
}

UIOperation::~UIOperation()
{
    delete m_msgs;
    //delete m_swapMsgs;
}

Message* UIOperation::obtain()
{
    //AutoLock lock(&m_uiMutex);
    return MessageCache::obtain();
}

LVoid UIOperation::opSetText(LVoid* view, const String& text)
{
    Message* msg = obtain();
    msg->type = UIOP_SETTEXT;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = (LIntPtr)view;
    m_msgs->addElement(msg);
}

LVoid UIOperation::opSetInput(LVoid* view, const String& text)
{
    Message* msg = obtain();
    msg->type = UIOP_SETINPUT;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = (LIntPtr)view;
    m_msgs->addElement(msg);
}

LVoid UIOperation::opAddChild(LVoid* view, LVoid* child)
{
    Message* msg = obtain();
    msg->type = UIOP_ADDCHILD;
    msg->obj = view;
    msg->arg0 = (LIntPtr)child;
    m_msgs->addElement(msg);
}

LVoid UIOperation::opSetImageUrl(LVoid* view, const String& url)
{
    Message* msg = obtain();
    msg->type = UIOP_SETIMAGE_URL;
    msg->obj = url.GetBuffer();
    msg->arg0 = url.GetLength();
    msg->arg1 = (LIntPtr)view;
    m_msgs->addElement(msg);
}

LVoid UIOperation::opLoadImageUrl(LVoid* view, const String& url)
{
    Message* msg = obtain();
    msg->type = UIOP_LOADIMAGE_URL;
    msg->obj = url.GetBuffer();
    msg->arg0 = url.GetLength();
    msg->arg1 = (LIntPtr)view;
    m_msgs->addElement(msg);
}

LVoid UIOperation::opViewDraw(LVoid* view)
{
    Message* msg = obtain();
    msg->type = UIOP_DRAW;
    msg->obj = view;
    m_msgs->addElement(msg);
}

LVoid UIOperation::opApplyDomStyle(LVoid* view)
{
    Message* msg = obtain();
    msg->type = UIOP_APPLY_DOM_STYLE;
    msg->obj = view;
    m_msgs->addElement(msg);
}

/*
LVoid UIOperation::swapBufferImpl()
{
    //AutoLock lock(&m_uiMutex);

    KVector<Message*>* buffer = m_msgs;
    m_msgs = m_swapMsgs;
    m_swapMsgs = buffer;
    m_msgs->clear();
}
*/

// 交换buffer，m_swapMsgs指针作为绘制时使用
LVoid UIOperation::swapBuffer()
{
    //swapBufferImpl();
    UIThread::instance()->uiExecute();
}

LVoid UIOperation::execute()
{
    // 处理Widget DOM相关操作
    //AutoLock lock(&m_uiMutex);
    LInt size = m_msgs->size();
    for (LInt index = 0; index < size; ++index) {
        Message* msg = m_msgs->elementAt(index);
        if (!msg)
            continue;
        switch (msg->type) {
        case UIOP_ADDCHILD:
            viewAddChild(msg);
            break;
        case UIOP_SETINPUT:
            viewSetInput(msg);
            break;
        case UIOP_SETTEXT:
            viewSetText(msg);
            break;
        case UIOP_SETIMAGE_URL: {
            String url(_CS(msg->obj), LTrue, msg->arg0);
            ImageView* view = (ImageView*)msg->arg1;
            view->setUrl(url);
        } break;
        case UIOP_LOADIMAGE_URL: {
            String url(_CS(msg->obj), LTrue, msg->arg0);
            ImageView* view = (ImageView*)msg->arg1;
            view->loadImage(url);
        } break;
        case UIOP_DRAW: {
            viewDraw(msg);
        } break;
        case UIOP_APPLY_DOM_STYLE: {
            HtmlView* root = (HtmlView*)msg->obj;
            ResourceLoader* loader = UIView::current()->getLoader();
            root->setStyle(loader->render()->getStyleManager(), kBoyiaNull);
        } break;
        }

        msg->msgRecycle();
    }

    m_msgs->clear();
}

LVoid UIOperation::viewSetInput(Message* msg)
{
    if (!msg->arg1) {
        return;
    }
    String text(_CS(msg->obj), LTrue, msg->arg0);
    reinterpret_cast<InputView*>(msg->arg1)->setInputValue(text);
}

LVoid UIOperation::viewSetText(Message* msg)
{
    String text(_CS(msg->obj), LTrue, msg->arg0);
    HtmlView* view = (HtmlView*)msg->arg1;

    if (view->m_children.count()) {
        HtmlView* firstChild = *view->m_children.begin();
        if (firstChild && firstChild->isText()) {
            TextView* item = (yanbo::TextView*)firstChild;
            item->setText(text);
        }
    } else {
        TextView* item = new TextView(_CS(""), text);
        item->setParent(view);
        view->addChild(item);

        ResourceLoader* loader = UIView::current()->getLoader();
        view->setStyle(loader->render()->getStyleManager(), kBoyiaNull);
    }
}

LVoid UIOperation::viewAddChild(Message* msg)
{
    HtmlView* view = static_cast<HtmlView*>(msg->obj);
    HtmlView* child = (HtmlView*)msg->arg0;
    if (view) {
        child->setParent(view);
        view->addChild(child);
    }
}

LVoid UIOperation::viewDraw(Message* msg)
{
    HtmlView* view = (HtmlView*)msg->obj;
    if (!view)
        return;
    view->layout();
    LGraphicsContext* gc = UIView::current()->getGraphicsContext();
    view->paint(*gc);
}
}

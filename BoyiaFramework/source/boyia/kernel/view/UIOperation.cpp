#include "UIOperation.h"
#include "HtmlView.h"
#include "ImageView.h"
#include "InputView.h"
#include "TextView.h"
#include "UIThread.h"
#include "UIView.h"
#include "WeakPtr.h"

namespace yanbo {
#define kMaxOpMessageSize 50

UIOperation::UIOperation()
    : m_msgs(new KVector<Message*>(0, kMaxOpMessageSize))
{
}

UIOperation::~UIOperation()
{
    delete m_msgs;
}

Message* UIOperation::obtain()
{
    return MessageCache::obtain();
}

LVoid UIOperation::opSetText(HtmlView* view, const String& text)
{
    Message* msg = obtain();
    msg->type = UIOP_SETTEXT;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = (LIntPtr)(new WeakPtr<HtmlView>(view));
    m_msgs->addElement(msg);
}

LVoid UIOperation::opSetInput(HtmlView* view, const String& text)
{
    Message* msg = obtain();
    msg->type = UIOP_SETINPUT;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = (LIntPtr)(new WeakPtr<HtmlView>(view));
    m_msgs->addElement(msg);
}

LVoid UIOperation::opAddChild(HtmlView* view, HtmlView* child)
{
    Message* msg = obtain();
    msg->type = UIOP_ADDCHILD;
    msg->obj = new WeakPtr<HtmlView>(view);
    msg->arg0 = (LIntPtr)(new WeakPtr<HtmlView>(child));
    m_msgs->addElement(msg);
}

LVoid UIOperation::opSetImageUrl(HtmlView* view, const String& url)
{
    Message* msg = obtain();
    msg->type = UIOP_SETIMAGE_URL;
    msg->obj = url.GetBuffer();
    msg->arg0 = url.GetLength();
    msg->arg1 = (LIntPtr)(new WeakPtr<HtmlView>(view));
    m_msgs->addElement(msg);
}

LVoid UIOperation::opLoadImageUrl(HtmlView* view, const String& url)
{
    Message* msg = obtain();
    msg->type = UIOP_LOADIMAGE_URL;
    msg->obj = url.GetBuffer();
    msg->arg0 = url.GetLength();
    msg->arg1 = (LIntPtr)(new WeakPtr<HtmlView>(view));
    m_msgs->addElement(msg);
}

LVoid UIOperation::opViewDraw(HtmlView* view)
{
    Message* msg = obtain();
    msg->type = UIOP_DRAW;
    msg->obj = new WeakPtr<HtmlView>(view);
    m_msgs->addElement(msg);
}

LVoid UIOperation::opApplyDomStyle(HtmlView* view)
{
    Message* msg = obtain();
    msg->type = UIOP_APPLY_DOM_STYLE;
    msg->obj = new WeakPtr<HtmlView>(view);
    m_msgs->addElement(msg);
}

LVoid UIOperation::swapBuffer()
{
    UIThread::instance()->uiExecute();
}

LVoid UIOperation::execute()
{
    // 处理Widget DOM相关操作
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
            WeakPtr<HtmlView>* view = reinterpret_cast<WeakPtr<HtmlView>*>(msg->arg1);
            if (*view) {
                static_cast<ImageView*>(view->get())->setUrl(url);
            }
            delete view;
        } break;
        case UIOP_LOADIMAGE_URL: {
            String url(_CS(msg->obj), LTrue, msg->arg0);
            WeakPtr<HtmlView>* view = reinterpret_cast<WeakPtr<HtmlView>*>(msg->arg1);
            if (*view) {
                static_cast<ImageView*>(view->get())->loadImage(url);
            }
            delete view;
        } break;
        case UIOP_DRAW: {
            viewDraw(msg);
        } break;
        case UIOP_APPLY_DOM_STYLE: {
            WeakPtr<HtmlView>* root = static_cast<WeakPtr<HtmlView>*>(msg->obj);
            if (*root) {
                ResourceLoader* loader = UIView::current()->getLoader();
                (*root)->setStyle(loader->render()->getStyleManager(), kBoyiaNull);
            }

            delete root;
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
    WeakPtr<InputView>* inputView = reinterpret_cast<WeakPtr<InputView>*>(msg->arg1);
    if (*inputView) {
        (*inputView)->setInputValue(text);
    }
    delete inputView;
}

LVoid UIOperation::viewSetText(Message* msg)
{
    String text(_CS(msg->obj), LTrue, msg->arg0);
    WeakPtr<HtmlView>* view = reinterpret_cast<WeakPtr<HtmlView>*>(msg->arg1);
    if (!view->get()) {
        delete view;
        return;
    }

    if ((*view)->m_children.count()) {
        HtmlView* firstChild = (*(*view)->m_children.begin());
        if (firstChild && firstChild->isText()) {
            TextView* item = static_cast<yanbo::TextView*>(firstChild);
            item->setText(text);
        }
    } else {
        TextView* item = new TextView(_CS(""), text);
        item->setParent(*view);
        (*view)->addChild(item);

        ResourceLoader* loader = UIView::current()->getLoader();
        (*view)->setStyle(loader->render()->getStyleManager(), kBoyiaNull);
    }

    delete view;
}

LVoid UIOperation::viewAddChild(Message* msg)
{
    WeakPtr<HtmlView>* view = static_cast<WeakPtr<HtmlView>*>(msg->obj);
    WeakPtr<HtmlView>* child = reinterpret_cast<WeakPtr<HtmlView>*>(msg->arg0);
    if (*view && *child) {
        (*child)->setParent(*view);
        (*view)->addChild(*child);
    }

    delete view;
    delete child;
}

LVoid UIOperation::viewDraw(Message* msg)
{
    WeakPtr<HtmlView>* view = static_cast<WeakPtr<HtmlView>*>(msg->obj);
    if (*view) {
        (*view)->layout();
        LGraphicsContext* gc = UIView::current()->getGraphicsContext();
        (*view)->paint(*gc);
    }
   
    delete view;
}
}

#include "UIOperation.h"
#include "HtmlView.h"
#include "TextView.h"
#include "ImageView.h"
#include "UIView.h"
#include "MiniThread.h"

namespace yanbo
{
#define MAX_OPMSG_SIZE 50
UIOperation* UIOperation::s_operation = NULL;

UIOperation* UIOperation::instance()
{
    if (s_operation == NULL)
    {
    	s_operation = new UIOperation();
    }

    return s_operation;
}

UIOperation::UIOperation()
    : m_msgs(new KVector<MiniMessage*>(0, MAX_OPMSG_SIZE))
    , m_swapMsgs(new KVector<MiniMessage*>(0, MAX_OPMSG_SIZE))
{
}

UIOperation::~UIOperation()
{
	delete m_msgs;
	delete m_swapMsgs;
}

MiniMessage* UIOperation::obtain()
{
	AutoLock lock(&m_uiMutex);
	return MiniMessageCache::obtain();
}

LVoid UIOperation::opSetText(LVoid* view, const String& text)
{
    MiniMessage* msg = obtain();
    msg->type = UIOP_SETTEXT;
	msg->obj = text.GetBuffer();
	msg->arg0 = text.GetLength();
	msg->arg1 = (LIntPtr) view;
	m_msgs->addElement(msg);
}

LVoid UIOperation::opAddChild(LVoid* view, LVoid* child)
{
	MiniMessage* msg = obtain();
	msg->type = UIOP_ADDCHILD;
	msg->obj = view;
	msg->arg0 = (LIntPtr) child;
	m_msgs->addElement(msg);
}

LVoid UIOperation::opSetImageUrl(LVoid* view, const String& url)
{
	MiniMessage* msg = obtain();
	msg->type = UIOP_SETIMAGE_URL;
	msg->obj = url.GetBuffer();
	msg->arg0 = url.GetLength();
	msg->arg1 = (LIntPtr) view;
	m_msgs->addElement(msg);
}

LVoid UIOperation::opLoadImageUrl(LVoid* view, const String& url)
{
	MiniMessage* msg = obtain();
	msg->type = UIOP_LOADIMAGE_URL;
	msg->obj = url.GetBuffer();
	msg->arg0 = url.GetLength();
	msg->arg1 = (LIntPtr) view;
	m_msgs->addElement(msg);
}

LVoid UIOperation::opViewDraw(LVoid* view)
{
	MiniMessage* msg = obtain();
	msg->type = UIOP_DRAW;
	msg->obj = view;
	m_msgs->addElement(msg);
}

LVoid UIOperation::opApplyDomStyle(LVoid* view)
{
	MiniMessage* msg = obtain();
	msg->type = UIOP_APPLY_DOM_STYLE;
	msg->obj = view;
	m_msgs->addElement(msg);
}

// 交换buffer，m_swapMsgs指针作为绘制时使用
LVoid UIOperation::swapBuffer()
{
	{
		AutoLock lock(&m_uiMutex);
		if (m_swapMsgs->size())
		{
			LInt size = m_msgs->size();
			for (LInt index = 0; index < size; ++index)
			{
				m_swapMsgs->addElement(m_msgs->elementAt(index));
			}

			m_msgs->clear();
		}
		else
		{
			KVector<MiniMessage*>* buffer = m_msgs;
			m_msgs = m_swapMsgs;
			m_swapMsgs = buffer;
			m_msgs->clear();

			PaintThread::instance()->uiExecute();
		}
	}
}

LVoid UIOperation::execute()
{
    // 处理Widget DOM相关操作
	AutoLock lock(&m_uiMutex);
	LInt size = m_swapMsgs->size();
	for (LInt index = 0; index < size; ++index)
	{
		MiniMessage* msg = m_swapMsgs->elementAt(index);
		if (!msg) continue;
		switch (msg->type)
		{
		case UIOP_ADDCHILD:
			viewAddChild(msg);
		    break;
		case UIOP_SETINPUT:
		    break;
		case UIOP_SETTEXT:
			viewSetText(msg);
		    break;
		case UIOP_SETIMAGE_URL:
		    {
		    	String url(_CS(msg->obj), LTrue, msg->arg0);
		    	ImageView* view = (ImageView*) msg->arg1;
		    	view->setUrl(url);
		    }
			break;
		case UIOP_LOADIMAGE_URL:
		    {
		    	String url(_CS(msg->obj), LTrue, msg->arg0);
		    	ImageView* view = (ImageView*) msg->arg1;
		    	view->loadImage(url);
		    }
		    break;
		case UIOP_DRAW:
		    {
		    	viewDraw(msg);
		    }
		    break;
		case UIOP_APPLY_DOM_STYLE:
		    {
		    	HtmlView* root = (HtmlView*)msg->obj;
		    	ResourceLoader* loader = UIView::getInstance()->getLoader();
		        root->setStyle(loader->render()->getCssManager(), NULL);
		    }
		    break;
		}

		msg->msgRecycle();
	}

	m_swapMsgs->clear();
}

LVoid UIOperation::viewSetText(MiniMessage* msg)
{
	String text(_CS(msg->obj), LTrue, msg->arg0);
	HtmlView* view = (HtmlView*) msg->arg1;

	if (view->m_children.count()) {
		HtmlView* firstChild = *view->m_children.begin();
		if (firstChild && firstChild->isText()) {
			TextView* item = (yanbo::TextView*) firstChild;
			item->setText(text);
		}
	} else {
		TextView* item = new TextView(_CS(""), text, LFalse);
		item->setParent(view);
		view->addChild(item);

		ResourceLoader* loader = UIView::getInstance()->getLoader();
		view->setStyle(loader->render()->getCssManager(), NULL);
    }
}

LVoid UIOperation::viewAddChild(MiniMessage* msg)
{
    HtmlView* view = static_cast<HtmlView*>(msg->obj);
    HtmlView* child = (HtmlView*)msg->arg0;
    if (view)
    {
    	child->setParent(view);
    	view->addChild(child);
    }
}

LVoid UIOperation::viewDraw(MiniMessage* msg)
{
	HtmlView* view = (HtmlView*)msg->obj;
	if (!view) return;
	view->layout();
	LGraphicsContext* gc = UIView::getInstance()->getGraphicsContext();
	view->paint(*gc);
}
}

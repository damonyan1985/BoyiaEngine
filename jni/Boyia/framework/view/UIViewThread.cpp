#include "UIViewThread.h"
#include "UIView.h"
#include "NetworkBase.h"
#include "LGdi.h"

namespace yanbo
{
UIViewThread* UIViewThread::s_instance = NULL;

UIViewThread* UIViewThread::instance()
{
    if (!s_instance)
    {
    	s_instance = new UIViewThread();
    	s_instance->start();
    }

    return s_instance;
}

UIViewThread::UIViewThread()
{
}

// 该线程处理MJS引擎和UI引擎的操作，
// 分离GL渲染中的纹理生成和渲染，
// 纹理的创建和生成将会在本线程中，
// GL渲染线程PaintThread只负责绘制，
// 避免多线程同步引起的代码结构紊乱--
// 和性能降低问题
void UIViewThread::handleMessage(MiniMessage* msg)
{
	switch (msg->type)
	{
	case UIView_INIT:
		{
			String url(_CS(msg->obj), LTrue,  msg->arg0);
			UIView::getInstance()->loadPage(url);
		}
		break;
	case UIView_DRAW:
		{
			HtmlView* view = (HtmlView*) msg->arg0;
			UIView::getInstance()->getLoader()->repaint(view);
		}
		break;
	case UIView_QUIT:
		{
			UIView::getInstance()->destroy();
			m_continue = LFalse;
		}
		break;
	case UIView_KEYEVENT:
		{
			LKeyEvent* evt = (LKeyEvent*)msg->obj;
			UIView::getInstance()->handleKeyEvent(*evt);
			delete evt;
		}
		break;
	case UIView_LOAD_FINISHED:
		{
			String data((LUint8*)msg->obj, LTrue, msg->arg0);
			NetworkClient* client = (NetworkClient*)msg->arg1;
			if (client)
			{
				client->onLoadFinished(data);
			}
		}
		break;
	case UIView_LOAD_ERROR:
		{
			yanbo::NetworkClient* client = (yanbo::NetworkClient*)msg->arg1;
			if (client)
			{
				client->onLoadError(msg->arg0);
			}
		}
		break;
	case UIView_IMAGE_LOADED:
		{
			if (!msg->arg0) return;
			LImage* image = (LImage*) msg->arg0;
			image->setLoaded(LTrue);
		}
		break;
//	case UIView_TOUCH_EVENT:
//		{
//			LTouchEvent* evt = (LTouchEvent*)msg->obj;
//			UIView::getInstance()->handleTouchEvent(*evt);
//			delete evt;
//		}
//		break;
	}
}

void UIViewThread::destroy()
{
	MiniMessage* msg = m_queue->obtain();
	msg->type = UIView_QUIT;

	m_queue->push(msg);
	notify();
}

void UIViewThread::load(const String& url)
{
	MiniMessage* msg = m_queue->obtain();
	msg->type = UIView_INIT;
	msg->obj = url.GetBuffer();
	msg->arg0 = url.GetLength();

	m_queue->push(msg);
	notify();
}

void UIViewThread::loadFinished(const String& data, LInt callback)
{
	MiniMessage* msg = m_queue->obtain();
    msg->type = UIView_LOAD_FINISHED;
    msg->obj = data.GetBuffer();
	msg->arg0 = data.GetLength();
	msg->arg1 = callback;

	m_queue->push(msg);
	notify();
}

void UIViewThread::loadError(LInt callback, LInt error)
{
	MiniMessage* msg = m_queue->obtain();
	msg->type = UIView_LOAD_ERROR;
    msg->arg0 = error;
	msg->arg1 = callback;

	m_queue->push(msg);
	notify();
}

void UIViewThread::imageLoaded(LInt item)
{
	MiniMessage* msg = m_queue->obtain();
	msg->type = UIView_IMAGE_LOADED;
    msg->arg0 = item;

	m_queue->push(msg);
	notify();
}

void UIViewThread::handleKeyEvent(LKeyEvent* evt)
{
	MiniMessage* msg = m_queue->obtain();
	msg->type = UIView_KEYEVENT;
    msg->obj = evt;

	m_queue->push(msg);
	notify();
}
//void UIViewThread::handleTouchEvent(LTouchEvent* evt)
//{
//	MiniMessage* msg = m_queue->obtain();
//	msg->type = UIView_TOUCH_EVENT;
//    msg->obj = evt;
//
//	m_queue->push(msg);
//	notify();
//}

void UIViewThread::draw(LInt item)
{
	MiniMessage* msg = m_queue->obtain();
	msg->type = UIView_DRAW;
	msg->arg0 = item;
	m_queue->push(msg);
	notify();
}
}

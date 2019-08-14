#include "AppThread.h"
#include "UIView.h"
//#include "NetworkBase.h"
//#include "LGdi.h"

namespace yanbo {
AppEvent::~AppEvent()
{
}
LVoid AppEvent::execute()
{
    run();
    delete this;
}

AppThread::AppThread()
{
    start();
}

AppThread* AppThread::instance()
{
    static AppThread sThread;
    return &sThread;
}

LVoid AppThread::handleMessage(MiniMessage* msg)
{
    switch (msg->type) {
    case BOYIA_INIT: {
        String url(_CS(msg->obj), LTrue, msg->arg0);
        UIView::getInstance()->loadPage(url);
    } break;
    case BOYIA_QUIT: {
        //UIView::getInstance()->destroy();
        m_continue = LFalse;
    } break;
    case BOYIA_SEND_EVENT: {
        if (!msg->obj)
            return;
        static_cast<AppEvent*>(msg->obj)->execute();
    } break;
    }
}

LVoid AppThread::destroy()
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = BOYIA_QUIT;

    m_queue->push(msg);
    notify();
}

LVoid AppThread::load(const String& url)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = BOYIA_INIT;
    msg->obj = url.GetBuffer();
    msg->arg0 = url.GetLength();

    m_queue->push(msg);
    notify();
}

LVoid AppThread::sendEvent(AppEvent* event)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = BOYIA_SEND_EVENT;
    msg->obj = event;
    m_queue->push(msg);
    notify();
}
}

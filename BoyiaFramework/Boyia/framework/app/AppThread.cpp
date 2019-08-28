#include "AppThread.h"
#include "AppManager.h"
#include "SalLog.h"
#include "UIView.h"

namespace yanbo {
AppEvent::~AppEvent()
{
}
LVoid AppEvent::execute()
{
    run();
    delete this;
}

AppThread::AppThread(AppManager* manager)
    : m_manager(manager)
{
    start();
}

AppThread* AppThread::instance()
{
    return AppManager::instance()->appThread();
}

LVoid AppThread::handleMessage(MiniMessage* msg)
{
    switch (msg->type) {
    case kAppInit: {
        String url(_CS(msg->obj), LFalse, msg->arg0);
        m_manager->currentApp()->vm()->useVM();
        m_manager->currentApp()->view()->loadPage(url);
    } break;
    case kAppQuit: {
        m_continue = LFalse;
    } break;
    case kAppEvent: {
        if (!msg->obj)
            return;
        static_cast<AppEvent*>(msg->obj)->execute();
    } break;
    }
}

LVoid AppThread::destroy()
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kAppQuit;

    m_queue->push(msg);
    notify();
}

LVoid AppThread::load(const String& url)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kAppInit;
    msg->obj = url.GetBuffer();
    msg->arg0 = url.GetLength();

    m_queue->push(msg);
    notify();
}

LVoid AppThread::sendEvent(AppEvent* event)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kAppEvent;
    msg->obj = event;
    m_queue->push(msg);
    notify();
}
}

#include "UIThread.h"
//#include "Animation.h"
#include "AppManager.h"
#include "AutoLock.h"
//#include "GraphicsContextGL.h"
#include "HtmlView.h"
#include "InputView.h"
#include "KList.h"
//#include "MatrixState.h"
#include "Lock.h"
#include "OwnerPtr.h"
#include "PixelRatio.h"
#include "SalLog.h"
#include "UIOperation.h"
#include "UIThreadClientMap.h"
#include "UIView.h"
#include "VideoView.h"
#include "VsyncWaiter.h"
#include "WeakPtr.h"
#include <functional>

namespace yanbo {

UIEvent::~UIEvent()
{
}

LVoid UIEvent::execute()
{
    run();
}

class UIClosureEvent {
public:
    UIClosureEvent(const UIClosure& func)
        : m_func(func)
    {
    }

    UIClosure m_func;
};

UIThread::UIThread(AppManager* manager)
    : m_manager(manager)
    , m_gc(LGraphicsContext::create())
#if ENABLE(BOYIA_UI_VSYNC)
    , m_vsyncWaiter(VsyncWaiter::createVsyncWaiter())
#endif
{
    start();
}

UIThread::~UIThread()
{
    delete m_gc;
}

UIThread* UIThread::instance()
{
    // C++11单例高并发处理方法
    return yanbo::AppManager::instance()->uiThread();
}

LGraphicsContext* UIThread::graphics() const
{
    return m_gc;
}

LVoid UIThread::draw(LVoid* item)
{
    Message* msg = obtain();
    msg->type = kUiDraw;
    msg->obj = item;
    postMessage(msg);
}

LVoid UIThread::drawOnly(LVoid* item)
{
    Message* msg = obtain();
    msg->type = kUiDrawOnly;
    msg->obj = item;
    postMessage(msg);
}

LVoid UIThread::submit()
{
    Message* msg = obtain();
    msg->type = kUiSubmit;
    postMessage(msg);
}

LVoid UIThread::destroy()
{
    Message* msg = obtain();
    msg->type = kUiDestory;
    postMessage(msg);
}

LVoid UIThread::initContext()
{
    Message* msg = obtain();
    msg->type = kUiInit;
    postMessage(msg);
}

LVoid UIThread::handleMessage(Message* msg)
{
    switch (msg->type) {
    case kUiInit: {
        m_gc->reset();
    } break;
    case kUiDraw: {
        drawUI(msg->obj);
    } break;
    case kUiDrawOnly: {
        yanbo::HtmlView* item = static_cast<yanbo::HtmlView*>(msg->obj);
        if (item) {
            item->paint(*m_gc);
        }
    } break;
    case kUiTouchEvent: {
        OwnerPtr<LTouchEvent> evt = static_cast<LTouchEvent*>(msg->obj);
        reinterpret_cast<UIView*>(msg->arg0)->handleTouchEvent(*evt);
        flush();
    } break;
    case kUiKeyEvent: {
        OwnerPtr<LKeyEvent> evt = static_cast<LKeyEvent*>(msg->obj);
        reinterpret_cast<UIView*>(msg->arg0)->handleKeyEvent(*evt);
    } break;
    case kUiSetInput: {
        String text(_CS(msg->obj), LTrue, msg->arg0);
        
        Editor* editor = m_manager->currentApp()->view()->getEditor();
        InputView* view = static_cast<InputView*>(editor->view());
        //InputView* view = static_cast<InputView*>(reinterpret_cast<Editor*>(msg->arg1)->view()); //(InputView*)msg->arg1;
        if (!view) {
            return;
        }

        view->setInputValue(text);
        if (view->isEditor()) {
            view->setInputCursor(msg->arg1);
        }
        drawUI(view);
    } break;
    case kUiVideoUpdate: {
        drawUI((LVoid*)msg->arg0);
    } break;
    case kUiClientCallback: {
        if (!msg->arg0)
            return;
        UIThreadClientMap::instance()->clientCallback(msg->arg0);
    } break;
    case kUiOperationExec: {
        m_manager->currentApp()->view()->operation()->execute();
        flush();
    } break;
    case kUiSubmit: {
        flush();
    } break;
    case kUiOnKeyboardShow: {
        //InputView* view = static_cast<InputView*>(reinterpret_cast<Editor*>(msg->arg0)->view());
        Editor* editor = m_manager->currentApp()->view()->getEditor();
        InputView* view = static_cast<InputView*>(editor->view());
        if (!view) {
            return;
        }

        LayoutPoint topLeft = view->getAbsoluteContainerTopLeft();
        LInt y = topLeft.iY + view->getYpos();
        BOYIA_LOG("InputView y=%d and keyboardHeight=%d", y, msg->arg1);
        if (y < PixelRatio::logicHeight() - msg->arg1) {
            return;
        }

        HtmlView* rootView = view->getDocument()->getRenderTreeRoot();
        rootView->setYpos(rootView->getYpos() - msg->arg1);
        rootView->paint(*m_gc);
        flush();
    } break;
    case kUiOnKeyboardHide: {
        //InputView* view = static_cast<InputView*>(reinterpret_cast<Editor*>(msg->arg0)->view());
        Editor* editor = m_manager->currentApp()->view()->getEditor();
        InputView* view = static_cast<InputView*>(editor->view());
        if (!view) {
            return;
        }

        HtmlView* rootView = static_cast<HtmlView*>(view)->getDocument()->getRenderTreeRoot();
        if (rootView->getYpos() == 0) {
            return;
        }

        rootView->setYpos(rootView->getYpos() + msg->arg1);
        rootView->paint(*m_gc);
        flush();
    } break;
    case kUiDestory: {
        //m_context.destroyGL();
        quit();
    } break;
    case kUiReset: {
        reset(msg);
    } break;
    case kUiClosureTask: {
        OwnerPtr<UIClosureEvent> event = static_cast<UIClosureEvent*>(msg->obj);
        event->m_func();
    } break;
    case kUiEvent: {
        OwnerPtr<UIEvent> event = static_cast<UIEvent*>(msg->obj);
        event->execute();
    } break;
    case kUiInitApp: {
        String entry(_CS(msg->obj), LFalse, msg->arg0);
        m_manager->currentApp()->init(entry);
    } break;
    case kVsyncDraw: {
#if ENABLE(BOYIA_UI_VSYNC)
        m_gc->submit();
#endif
    } break;
    case kPlatformViewUpdate: {
#if ENABLE(BOYIA_PLATFORM_VIEW)
        String id(_CS(msg->obj), LTrue, msg->arg0);
        HtmlView* view = m_manager->currentApp()->view()->getDocument()->getItemByID(id);
        if (!view) {
            return;
        }

        drawUI(view);
#endif
    } break;
    case kFlingEvent: {
        // TODO handle fling event
        OwnerPtr<LFlingEvent> evt = static_cast<LFlingEvent*>(msg->obj);
        AppManager::instance()->currentApp()->view()->handleFlingEvent(*evt);
    } break;
    }
}

LVoid UIThread::flush()
{
#if ENABLE(BOYIA_UI_VSYNC)
    m_vsyncWaiter->awaitVSync();
#else
    m_gc->submit();
#endif
}

LVoid UIThread::setInputText(const String& text, LIntPtr item)
{
    Message* msg = obtain();
    msg->type = kUiSetInput;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = item;
    postMessage(msg);
}

void UIThread::videoUpdate(LIntPtr item)
{
    Message* msg = obtain();
    msg->type = kUiVideoUpdate;
    msg->arg0 = item;
    postMessage(msg);
}

LVoid UIThread::clientCallback(LIntPtr item)
{
    Message* msg = obtain();
    msg->type = kUiClientCallback;
    msg->arg0 = item;

    postMessage(msg);
}

LVoid UIThread::drawUI(LVoid* view)
{
    HtmlView* item = (HtmlView*)view;
    if (item) {
        item->paint(*m_gc);
    }

    flush();
}

LVoid UIThread::uiExecute()
{
    m_loop->queue()->removeMessage(kUiOperationExec);
    Message* msg = obtain();
    msg->type = kUiOperationExec;
    postMessage(msg);
}

LVoid UIThread::handleTouchEvent(LTouchEvent* evt)
{
    m_loop->queue()->removeMessage(kUiTouchEvent);
    Message* msg = obtain();
    msg->type = kUiTouchEvent;
    msg->obj = evt;
    msg->arg0 = reinterpret_cast<LIntPtr>(m_manager->currentApp()->view());

    postMessage(msg);
}

LVoid UIThread::handleKeyEvent(LKeyEvent* evt)
{
    Message* msg = obtain();
    msg->type = kUiKeyEvent;
    msg->obj = evt;
    msg->arg0 = reinterpret_cast<LIntPtr>(m_manager->currentApp()->view());

    postMessage(msg);
}

LVoid UIThread::onKeyboardShow(LIntPtr item, LInt keyboardHeight)
{
    Message* msg = obtain();
    msg->type = kUiOnKeyboardShow;
    msg->arg0 = item;
    msg->arg1 = keyboardHeight;
    postMessage(msg);
}

LVoid UIThread::onKeyboardHide(LIntPtr item, LInt keyboardHeight)
{
    Message* msg = obtain();
    msg->type = kUiOnKeyboardHide;
    msg->arg0 = item;
    msg->arg1 = keyboardHeight;
    postMessage(msg);
}

LVoid UIThread::resetContext()
{
    Message* msg = obtain();
    msg->obj = m_manager->currentApp()->view();
    msg->type = kUiReset;
    postMessage(msg);
}

LVoid UIThread::reset(Message* msg)
{
    m_gc->reset();
    flush();
}

LVoid UIThread::postClosureTask(const UIClosure& func)
{
    Message* msg = obtain();
    msg->type = kUiClosureTask;
    msg->obj = new UIClosureEvent(func);
    postMessage(msg);
}

LVoid UIThread::sendUIEvent(UIEvent* event)
{
    Message* msg = obtain();
    msg->type = kUiEvent;
    msg->obj = event;
    postMessage(msg);
}

LVoid UIThread::initApp(const String& entry)
{
    Message* msg = obtain();
    msg->type = kUiInitApp;
    msg->obj = entry.GetBuffer();
    msg->arg0 = entry.GetLength();

    postMessage(msg);
}

LVoid UIThread::vsyncDraw()
{
    Message* msg = obtain();
    msg->type = kVsyncDraw;
    postMessage(msg);
}

LVoid UIThread::platformViewUpdate(const String& id)
{
    Message* msg = obtain();
    msg->type = kPlatformViewUpdate;
    msg->obj = id.GetBuffer();
    msg->arg0 = id.GetLength();
    postMessage(msg);
}

LVoid UIThread::handleFlingEvent(LFlingEvent* evt)
{
    Message* msg = obtain();
    msg->type = kFlingEvent;
    msg->obj = evt;
    postMessage(msg);
}

LVoid VsyncWaiter::awaitVsyncForCallback(const Callback& callback)
{
    if (!callback) {
        return;
    }
    
    {
        AutoLock lock(&m_lock);
//        if (m_callback) {
//            return;
//        }
        m_callback = std::move(callback);
    }
    awaitVSync();
}

LVoid VsyncWaiter::fireCallback()
{
    Callback callback;
    {
        AutoLock lock(&m_lock);
        callback = std::move(m_callback);
    }
    
    if (!callback) {
        return;
    }
    
    UIThread::instance()->postClosureTask([callback] {
        callback();
    });
}
}

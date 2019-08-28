#include "UIThread.h"
#include "Animation.h"
#include "AppManager.h"
#include "AutoLock.h"
#include "GraphicsContextGL.h"
#include "HtmlView.h"
#include "InputView.h"
#include "KList.h"
//#include "MatrixState.h"
#include "MiniMutex.h"
#include "SalLog.h"
#include "ShaderUtil.h"
#include "UIOperation.h"
#include "UIView.h"
#include "VideoView.h"
#include <functional>

namespace yanbo {
UIThread::UIThread(AppManager* manager)
    : m_manager(manager)
    , m_gc(LGraphicsContext::create())
{
    start();
}

UIThread::~UIThread()
{
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
    MiniMessage* msg = obtain();
    msg->type = kUiDraw;
    msg->obj = item;
    postMessage(msg);
}

LVoid UIThread::drawOnly(LVoid* item)
{
    MiniMessage* msg = obtain();
    msg->type = kUiDrawOnly;
    msg->obj = item;
    postMessage(msg);
}

LVoid UIThread::submit()
{
    MiniMessage* msg = obtain();
    msg->type = kUiSubmit;
    postMessage(msg);
}

LVoid UIThread::destroy()
{
    MiniMessage* msg = obtain();
    msg->type = kUiDestory;
    postMessage(msg);
}

LVoid UIThread::initContext()
{
    MiniMessage* msg = obtain();
    msg->type = kUiInit;
    postMessage(msg);
}

LVoid UIThread::handleMessage(MiniMessage* msg)
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
        LTouchEvent* evt = static_cast<LTouchEvent*>(msg->obj);
        reinterpret_cast<UIView*>(msg->arg0)->handleTouchEvent(*evt);
        delete evt;
        flush();
    } break;
    case kUiKeyEvent: {
        LKeyEvent* evt = static_cast<LKeyEvent*>(msg->obj);
        reinterpret_cast<UIView*>(msg->arg0)->handleKeyEvent(*evt);
        delete evt;
    } break;
    case kUiSetInput: {
        String text(_CS(msg->obj), LTrue, msg->arg0);
        InputView* view = (InputView*)msg->arg1;
        view->setInputValue(text);
        drawUI(view);
    } break;
    case kUiVideoUpdate: {
        drawUI((LVoid*)msg->arg0);
    } break;
    case kUiImageLoaded: {
        if (!msg->arg0)
            return;
        reinterpret_cast<LImage*>(msg->arg0)->setLoaded(LTrue);
    } break;
    case kUiOperationExec: {
        UIOperation::instance()->execute();
        flush();
        //submit();
    } break;
    case kUiSubmit: {
        flush();
    } break;
    case kUiOnKeyboardShow: {
        HtmlView* view = reinterpret_cast<HtmlView*>(msg->arg0);
        LayoutPoint topLeft = view->getAbsoluteContainerTopLeft();
        LInt y = topLeft.iY + view->getYpos();
        KFORMATLOG("InputView y=%d and keyboardHeight=%d", y, msg->arg1);
        if (y < ShaderUtil::screenWidth() - msg->arg1) {
            return;
        }

        HtmlView* rootView = view->getDocument()->getRenderTreeRoot();

        rootView->setYpos(rootView->getYpos() - msg->arg1);

        rootView->paint(*m_gc);
        flush();
    } break;
    case kUiOnKeyboardHide: {
        HtmlView* view = reinterpret_cast<HtmlView*>(msg->arg0);
        HtmlView* rootView = view->getDocument()->getRenderTreeRoot();
        if (rootView->getYpos() == 0) {
            return;
        }

        rootView->setYpos(rootView->getYpos() + msg->arg1);
        rootView->paint(*m_gc);
        flush();
    } break;
    case kUiDestory: {
        //m_context.destroyGL();
        m_continue = LFalse;
    }
    case kUiReset: {
        resetGL(msg);
    } break;
    case kUiRunAnimation: {
        std::function<void()>* callback = (std::function<void()>*)msg->obj;
        (*callback)();
    } break;
    }
}

LVoid UIThread::flush()
{
    m_gc->submit();
}

LVoid UIThread::setInputText(const String& text, LIntPtr item)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kUiSetInput;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = item;
    m_queue->push(msg);
    notify();
}

void UIThread::videoUpdate(LIntPtr item)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kUiVideoUpdate;
    msg->arg0 = item;
    m_queue->push(msg);
    notify();
}

LVoid UIThread::imageLoaded(LIntPtr item)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kUiImageLoaded;
    msg->arg0 = item;

    m_queue->push(msg);
    notify();
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
    MiniMessage* msg = m_queue->obtain();
    msg->type = kUiOperationExec;
    m_queue->push(msg);
    notify();
}

LVoid UIThread::handleTouchEvent(LTouchEvent* evt)
{
    m_queue->removeMessage(kUiTouchEvent);
    MiniMessage* msg = m_queue->obtain();
    msg->type = kUiTouchEvent;
    msg->obj = evt;
    msg->arg0 = reinterpret_cast<LIntPtr>(m_manager->currentApp()->view());

    m_queue->push(msg);
    notify();
}

LVoid UIThread::handleKeyEvent(LKeyEvent* evt)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = kUiKeyEvent;
    msg->obj = evt;
    msg->arg0 = reinterpret_cast<LIntPtr>(m_manager->currentApp()->view());

    m_queue->push(msg);
    notify();
}

LVoid UIThread::onKeyboardShow(LIntPtr item, LInt keyboardHeight)
{
    MiniMessage* msg = obtain();
    msg->type = kUiOnKeyboardShow;
    msg->arg0 = item;
    msg->arg1 = keyboardHeight;
    postMessage(msg);
}

LVoid UIThread::onKeyboardHide(LIntPtr item, LInt keyboardHeight)
{
    MiniMessage* msg = obtain();
    msg->type = kUiOnKeyboardHide;
    msg->arg0 = item;
    msg->arg1 = keyboardHeight;
    postMessage(msg);
}

LVoid UIThread::resetContext()
{
    MiniMessage* msg = obtain();
    msg->obj = m_manager->currentApp()->view();
    msg->type = kUiReset;
    postMessage(msg);
}

LVoid UIThread::resetGL(MiniMessage* msg)
{
    m_gc->reset();
    UIView* view = (UIView*)msg->obj;
    drawUI(view->getDocument()->getRenderTreeRoot());
}

LVoid UIThread::runAnimation(LVoid* callback)
{
    MiniMessage* msg = obtain();
    msg->type = kUiRunAnimation;
    msg->obj = callback;
    postMessage(msg);
}
}

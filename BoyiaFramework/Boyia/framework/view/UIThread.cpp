#include "UIThread.h"
#include "Animation.h"
#include "AppManager.h"
#include "AutoLock.h"
#include "GraphicsContextGL.h"
#include "HtmlView.h"
#include "InputView.h"
#include "KList.h"
#include "MatrixState.h"
#include "MiniMutex.h"
#include "SalLog.h"
#include "ShaderUtil.h"
#include "UIOperation.h"
#include "UIView.h"
#include "VideoView.h"
#include <functional>

namespace yanbo {
UIThread::UIThread()
{
    m_gc = new util::GraphicsContextGL();
    start();
}

UIThread::~UIThread()
{
}

UIThread* UIThread::instance()
{
    // C++11单例高并发处理方法
    // static UIThread sThread;
    // return &sThread;
    return yanbo::AppManager::instance()->uiThread();
}

LGraphicsContext* UIThread::graphics() const
{
    return m_gc;
}

LVoid UIThread::draw(LVoid* item)
{
    MiniMessage* msg = obtain();
    msg->type = UI_DRAW;
    msg->obj = item;
    postMessage(msg);
}

LVoid UIThread::drawOnly(LVoid* item)
{
    MiniMessage* msg = obtain();
    msg->type = UI_DRAWONLY;
    msg->obj = item;
    postMessage(msg);
}

LVoid UIThread::submit()
{
    // if (m_queue->hasMessage(UI_SUBMIT))
    // {
    //     return;
    // }

    MiniMessage* msg = obtain();
    msg->type = UI_SUBMIT;
    postMessage(msg);
}

LVoid UIThread::destroy()
{
    MiniMessage* msg = obtain();
    msg->type = UI_DESTROY;
    postMessage(msg);
}

LVoid UIThread::initContext(LVoid* win)
{
    m_context.setWindow(win);
    MiniMessage* msg = obtain();
    msg->type = UI_INIT;
    postMessage(msg);
}

LVoid UIThread::handleMessage(MiniMessage* msg)
{
    switch (msg->type) {
    case UI_INIT: {
        initGL();
    } break;
    case UI_DRAW: {
        drawUI(msg->obj);
    } break;
    case UI_DRAWONLY: {
        yanbo::HtmlView* item = static_cast<yanbo::HtmlView*>(msg->obj);
        if (item) {
            item->paint(*m_gc);
        }
    } break;
    case UI_TOUCH_EVENT: {
        LTouchEvent* evt = static_cast<LTouchEvent*>(msg->obj);
        UIView::getInstance()->handleTouchEvent(*evt);
        delete evt;
        //submit();
        flush();
    } break;
    case UI_KEY_EVENT: {
        LKeyEvent* evt = static_cast<LKeyEvent*>(msg->obj);
        UIView::getInstance()->handleKeyEvent(*evt);
        delete evt;
    } break;
    case UI_SETINPUT: {
        String text(_CS(msg->obj), LTrue, msg->arg0);
        InputView* view = (InputView*)msg->arg1;
        view->setInputValue(text);
        drawUI(view);
    } break;
    case UI_VIDEO_UPDATE: {
        drawUI((LVoid*)msg->arg0);
    } break;
    case UI_IMAGE_LOADED: {
        if (!msg->arg0)
            return;
        reinterpret_cast<LImage*>(msg->arg0)->setLoaded(LTrue);
    } break;
    case UI_OP_EXEC: {
        UIOperation::instance()->execute();
        flush();
        //submit();
    } break;
    case UI_SUBMIT: {
        flush();
    } break;
    case UI_ON_KEYBOARD_SHOW: {
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
    case UI_ON_KEYBOARD_HIDE: {
        HtmlView* view = reinterpret_cast<HtmlView*>(msg->arg0);
        HtmlView* rootView = view->getDocument()->getRenderTreeRoot();
        if (rootView->getYpos() == 0) {
            return;
        }

        rootView->setYpos(rootView->getYpos() + msg->arg1);
        rootView->paint(*m_gc);
        flush();
    } break;
    case UI_DESTROY: {
        m_context.destroyGL();
        m_continue = LFalse;
    }
    case UI_RESET: {
        resetGL();
    } break;
    case UI_RUN_ANIM: {
        std::function<void()>* callback = (std::function<void()>*)msg->obj;
        (*callback)();
    } break;
    }
}

LVoid UIThread::initGL()
{
    //int width = yanbo::UIView::getInstance()->getClientRange().GetWidth();
    //int height = yanbo::UIView::getInstance()->getClientRange().GetHeight();

    MiniTextureCache::getInst()->clear();
    //GLContext::initGLContext(GLContext::EWindow);
    m_context.initGL(GLContext::EWindow);
    glViewport(0, 0, m_context.viewWidth(), m_context.viewHeight());
    ShaderUtil::setRealScreenSize(m_context.viewWidth(), m_context.viewHeight());
    MatrixState::init();

    //glViewport(0, 0, width, height);
    //LReal32 ratio = (LReal32) width / height;
    //LReal32 ratio = (LReal32) height / width;
    LReal32 ratio = 1.0f;
    // 设置透视投影
    MatrixState::setProjectFrustum(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f, 1.0f, 50);
    //yanbo::MatrixState::setProjectOrtho(-1.0f * ratio, 1.0f * ratio, -1.0f, 1.0f, -1.0f, 1.0f);

    MatrixState::setCamera(0, 0, 1, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    MatrixState::copyMVMatrix();

    MatrixState::setInitStack();

    GLPainter::init();
}

LVoid UIThread::flush()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_gc->submit();
    m_context.postBuffer();
}

LVoid UIThread::setInputText(const String& text, LIntPtr item)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = UI_SETINPUT;
    msg->obj = text.GetBuffer();
    msg->arg0 = text.GetLength();
    msg->arg1 = item;
    m_queue->push(msg);
    notify();
}

void UIThread::videoUpdate(LIntPtr item)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = UI_VIDEO_UPDATE;
    msg->arg0 = item;
    m_queue->push(msg);
    notify();
}

LVoid UIThread::imageLoaded(LIntPtr item)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = UI_IMAGE_LOADED;
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
    //submit();
}

LVoid UIThread::uiExecute()
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = UI_OP_EXEC;
    m_queue->push(msg);
    notify();
}

LVoid UIThread::handleTouchEvent(LTouchEvent* evt)
{
    m_queue->removeMessage(UI_TOUCH_EVENT);
    MiniMessage* msg = m_queue->obtain();
    msg->type = UI_TOUCH_EVENT;
    msg->obj = evt;

    m_queue->push(msg);
    notify();
}

LVoid UIThread::handleKeyEvent(LKeyEvent* evt)
{
    MiniMessage* msg = m_queue->obtain();
    msg->type = UI_KEY_EVENT;
    msg->obj = evt;

    m_queue->push(msg);
    notify();
}

LVoid UIThread::onKeyboardShow(LIntPtr item, LInt keyboardHeight)
{
    MiniMessage* msg = obtain();
    msg->type = UI_ON_KEYBOARD_SHOW;
    msg->arg0 = item;
    msg->arg1 = keyboardHeight;
    postMessage(msg);
}

LVoid UIThread::onKeyboardHide(LIntPtr item, LInt keyboardHeight)
{
    MiniMessage* msg = obtain();
    msg->type = UI_ON_KEYBOARD_HIDE;
    msg->arg0 = item;
    msg->arg1 = keyboardHeight;
    postMessage(msg);
}

LVoid UIThread::resetContext(LVoid* win)
{
    m_context.setWindow(win);
    MiniMessage* msg = obtain();
    msg->type = UI_RESET;
    postMessage(msg);
}

LVoid UIThread::resetGL()
{
    initGL();
    drawUI(UIView::getInstance()->getDocument()->getRenderTreeRoot());
}

LVoid UIThread::runAnimation(LVoid* callback)
{
    MiniMessage* msg = obtain();
    msg->type = UI_RUN_ANIM;
    msg->obj = callback;
    postMessage(msg);
}
}

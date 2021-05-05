#include "BoyiaView.h"
#include "Animation.h"
//#include "AutoObject.h"
#include "BoyiaViewDoc.h"
//#include "JNIUtil.h"
#include "UIOperation.h"
#include "UIView.h"

using namespace yanbo;

namespace boyia {

BoyiaView::BoyiaView(BoyiaRuntime* runtime)
    : BoyiaBase(runtime)
    , m_item(kBoyiaNull)
    , m_type(0)
{
}

BoyiaView::~BoyiaView()
{
    if (m_item) {
        m_item->setListener(kBoyiaNull);
    }
}

LVoid BoyiaView::setX(LInt x)
{
    if (m_item) {
        m_item->getStyle()->left = x;
        m_item->setXpos(x);
    }
}

LVoid BoyiaView::setY(LInt y)
{
    if (m_item) {
        m_item->getStyle()->top = y;
        m_item->setYpos(y);
    }
}

LInt BoyiaView::left() const
{
    if (m_item) {
        return m_item->getXpos();
    }

    return 0;
}

LInt BoyiaView::top() const
{
    if (m_item) {
        return m_item->getYpos();
    }

    return 0;
}

LInt BoyiaView::width() const
{
    if (m_item) {
        return m_item->getWidth();
    }

    return 0;
}

LInt BoyiaView::height() const
{
    if (m_item) {
        return m_item->getHeight();
    }

    return 0;
}

LVoid BoyiaView::drawView()
{
    if (m_item) {
        runtime()->view()->operation()->opViewDraw(m_item);
    }
}

LVoid BoyiaView::commit()
{
    runtime()->view()->operation()->swapBuffer();
}

LVoid BoyiaView::setStyle(String& cls)
{
    if (m_item) {
        ResourceLoader* loader = runtime()->view()->getLoader();
        LGraphicsContext* gc = runtime()->view()->getGraphicsContext();
        m_item->setClassName(cls);
        m_item->setStyle(loader->render()->getStyleManager(), kBoyiaNull);

        // 判断z-index
        if (m_item->isPositioned() && m_item->getStyle()->zindex > 0) {
            m_item->getParent()->relayoutZIndexChild();
        }

        m_item->layout();

        loader->render()->paint(m_item);
    }
}

HtmlView* BoyiaView::item() const
{
    return m_item;
}

// 动画
LVoid BoyiaView::startOpacity(LInt opacity, LInt duration)
{
    OpacityAnimation* anim = new OpacityAnimation(m_item);
    anim->setDuration(duration);
    anim->setOpacity(opacity);

    Animator::instance()->startAnimation(anim);
}

LVoid BoyiaView::startScale(LInt scale, LInt duration)
{
    ScaleAnimation* anim = new ScaleAnimation(m_item);
    anim->setDuration(duration);
    anim->setScale(((float)scale) / 100);

    Animator::instance()->startAnimation(anim);
}

LVoid BoyiaView::startTranslate(const LPoint& point, LInt duration)
{
    TranslateAnimation* anim = new TranslateAnimation(m_item);
    anim->setDuration(duration);
    anim->setPosition(point);
    Animator::instance()->startAnimation(anim);
}

LVoid BoyiaView::onPressDown(LVoid* view)
{
    if (m_type & LTouchEvent::ETOUCH_DOWN) {
        KLOG("BoyiaBase::onPressDown");
        // 处理touchdown
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_DOWN >> 1];
        SaveLocalSize(m_runtime->vm());
        LocalPush(val, m_runtime->vm());
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj, m_runtime->vm());
    }
}
LVoid BoyiaView::onPressMove(LVoid* view)
{
    if (m_type & LTouchEvent::ETOUCH_MOVE) {
        // 处理touchmove
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_MOVE >> 1];

        SaveLocalSize(m_runtime->vm());
        LocalPush(val, m_runtime->vm());
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj, m_runtime->vm());
    }
}

LVoid BoyiaView::onPressUp(LVoid* view)
{
    BOYIA_LOG("BoyiaBase::onPressUp m_type=%d ETOUCH_UP=%d", m_type, LTouchEvent::ETOUCH_UP);
    if (m_type & LTouchEvent::ETOUCH_UP) {
        // 处理touchup
        BoyiaValue* val = &m_callbacks[LTouchEvent::ETOUCH_UP >> 1];
        SaveLocalSize(m_runtime->vm());
        LocalPush(val, m_runtime->vm());
        BoyiaValue* obj = m_boyiaView.mValue.mObj.mPtr == 0 ? kBoyiaNull : &m_boyiaView;
        NativeCall(obj, m_runtime->vm());
    }
}

LVoid BoyiaView::onKeyDown(LInt keyCode, LVoid* view)
{
}

LVoid BoyiaView::onKeyUp(LInt keyCode, LVoid* view)
{
}

LVoid BoyiaView::addListener(LInt type, BoyiaValue* callback)
{
    m_type |= type;
    KFORMATLOG("BoyiaBase::addListener m_type=%d", m_type);
    ValueCopy(&m_callbacks[type >> 1], callback);

    if (m_item && !m_item->getListener()) {
        m_item->setListener(this);
    }   
}

LVoid BoyiaView::setBoyiaView(BoyiaValue* value)
{
    ValueCopy(&m_boyiaView, value);
}
}

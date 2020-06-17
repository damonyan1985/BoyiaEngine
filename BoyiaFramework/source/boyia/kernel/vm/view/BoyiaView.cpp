#include "BoyiaView.h"
#include "Animation.h"
//#include "AutoObject.h"
#include "BoyiaViewDoc.h"
//#include "JNIUtil.h"
#include "UIOperation.h"
#include "UIView.h"

using namespace yanbo;

namespace boyia {
static LVoid startAnimtion(Animation* anim)
{
    AnimationTask* task = new AnimationTask();
    task->addAnimation(anim);
    Animator::instance()->runTask(task);
}

BoyiaView::BoyiaView(BoyiaRuntime* runtime)
    : BoyiaBase(runtime)
    , m_item(kBoyiaNull)
{
}

BoyiaView::~BoyiaView()
{
    if (m_item) {
        m_item->setListener(kBoyiaNull);
    }
}

LVoid BoyiaView::addListener(LInt type, BoyiaValue* callback)
{
    BoyiaBase::addListener(type, callback);
    if (!m_item->getListener()) {
        m_item->setListener(this);
    }
}

LVoid BoyiaView::setX(LInt x)
{
    m_item->getStyle()->left = x;
    m_item->setXpos(x);
}

LVoid BoyiaView::setY(LInt y)
{
    m_item->getStyle()->top = y;
    m_item->setYpos(y);
}

LInt BoyiaView::left() const
{
    return m_item->getXpos();
}

LInt BoyiaView::top() const
{
    return m_item->getYpos();
}

LInt BoyiaView::width() const
{
    return m_item->getWidth();
}

LInt BoyiaView::height() const
{
    return m_item->getHeight();
}

LVoid BoyiaView::drawView()
{
    //m_item->relayout();
    //UIView::getInstance()->getLoader()->render()->paint(m_item);
    runtime()->view()->operation()->opViewDraw(m_item);
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

    startAnimtion(anim);
}

LVoid BoyiaView::startScale(LInt scale, LInt duration)
{
    ScaleAnimation* anim = new ScaleAnimation(m_item);
    anim->setDuration(duration);
    anim->setScale(((float)scale) / 100);

    startAnimtion(anim);
}

LVoid BoyiaView::startTranslate(const LPoint& point, LInt duration)
{
    TranslateAnimation* anim = new TranslateAnimation(m_item);
    anim->setDuration(duration);
    anim->setPosition(point);
    startAnimtion(anim);
}
}

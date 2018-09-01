#include "JSView.h"
#include "JSViewDoc.h"
#include "JNIUtil.h"
#include "AutoObject.h"
#include "UIView.h"
#include "Animation.h"
#include "UIOperation.h"

using namespace yanbo;

namespace boyia
{
static LVoid startAnimtion(Animation* anim)
{
	AnimationTask* task = new AnimationTask();
	task->addAnimation(anim);
	AnimationThread::instance()->runTask(task);
}

JSView::JSView()
    : m_item(NULL)
{
}

JSView::~JSView()
{
	if (m_item)
	{
		m_item->setListener(NULL);
	}
}

void JSView::addListener(LInt type, BoyiaValue* callback)
{
	JSBase::addListener(type, callback);
    if (!m_item->getListener())
    {
    	m_item->setListener(this);
    }
}

void JSView::setX(LInt x)
{
	m_item->getStyle()->left = x;
	m_item->setXpos(x);
}

void JSView::setY(LInt y)
{
	m_item->getStyle()->top = y;
	m_item->setYpos(y);
}

LInt JSView::left() const
{
	return m_item->getXpos();
}

LInt JSView::top() const
{
	return m_item->getYpos();
}

LInt JSView::width() const
{
	return m_item->getWidth();
}

LInt JSView::height() const
{
	return m_item->getHeight();
}

void JSView::drawView()
{
	//m_item->relayout();
	//UIView::getInstance()->getLoader()->render()->paint(m_item);
	yanbo::UIOperation::instance()->opViewDraw(m_item);
}

void JSView::commit()
{
	yanbo::UIOperation::instance()->swapBuffer();
}

void JSView::setStyle(String& cls)
{
	if (m_item)
	{
		ResourceLoader* loader = UIView::getInstance()->getLoader();
		LGraphicsContext* gc = UIView::getInstance()->getGraphicsContext();
		m_item->setClassName(cls);
		m_item->setStyle(loader->render()->getCssManager(), NULL);

		// 判断z-index
		if (m_item->isPositioned() && m_item->getStyle()->zindex > 0)
		{
			m_item->getParent()->relayoutZIndexChild();
		}

		m_item->layout();

        loader->render()->paint(m_item);
	}
}

HtmlView* JSView::item() const
{
	return m_item;
}


// 动画
void JSView::startOpacity(LInt opacity, LInt duration)
{
	OpacityAnimation* anim = new OpacityAnimation(m_item);
	anim->setDuration(duration);
	anim->setOpacity(opacity);

	startAnimtion(anim);
}

void JSView::startScale(LInt scale, LInt duration)
{
	ScaleAnimation* anim = new ScaleAnimation(m_item);
	anim->setDuration(duration);
	anim->setScale(((float)scale)/100);

	startAnimtion(anim);
}

void JSView::startTranslate(const LPoint& point, LInt duration)
{
    TranslateAnimation* anim = new TranslateAnimation(m_item);
	anim->setDuration(duration);
	anim->setPosition(point);
	startAnimtion(anim);
}
}

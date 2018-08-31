/*
 * UIViewController.cpp
 *
 *  Created on: 2011-11-11
 *      Author: yanbo
 */

#include "UIViewController.h"
#include "UIView.h"

namespace yanbo
{

// 冒泡处理点击事件
static HtmlView* findViewByPosition(const LPoint& pt, HtmlView* view)
{
	HtmlViewList::Iterator iterEnd = view->m_children.end();
	HtmlViewList::Iterator iter = view->m_children.end();
	--iter;

	for (; iter != iterEnd; --iter)
	{
		// 为啥要从尾部向前进，考虑到zindex
		HtmlView* child = *iter;
		LayoutPoint topLeft = child->getAbsoluteContainerTopLeft();
        LInt x = child->getXpos() + topLeft.iX;
        LInt y = child->getYpos() + topLeft.iY;

    	KFORMATLOG("findViewByPosition x=%d, y=%d, w=%d, h=%d selectable=%d", x, y, child->getWidth(), child->getHeight(), child->isSelectable());
    	KFORMATLOG("findViewByPosition point x=%d, y=%d", pt.iX, pt.iY);

		if (pt.iX > x
            && pt.iX < x + child->getWidth()
			&& pt.iY > y
			&& pt.iY < y + child->getHeight())
		{

			if (child->isSelectable()) return child;
			return findViewByPosition(pt, child);
		}
	}

	return NULL;
}

UIViewController::UIViewController(UIView* view)
    : m_view(view)
    , m_target(NULL)
{
}

void UIViewController::onUpKey(LBool repeated)
{
	HtmlView* item = m_view->getDocument()->getCurrentItem();
	item->setSelected(LFalse);
	//item->setNeedPaint(LTrue);

	item = m_view->getDocument()->getPreItem();
	while (!item->isSelectable())
	{    
	    HtmlView* preItem = m_view->getDocument()->getPreItem();
	    
	    if (item == preItem)
	    {
	        break;
	    }
	    
	    item = preItem;
	}
	
	if (item->isSelectable())
	{
	    item->setSelected(LTrue);
	    
	    if (item->getYpos() < 0)
	    {
//	        int scrollYPos = item->getScrollYPos();
//	        scrollYPos -= scrollYPos - item->getYpos();
//	        m_view->getDocument()->getRenderTreeRoot()->setScrollPos(0, scrollYPos);
	    }
	}
		
	m_view->getLoader()->repaint();
}

void UIViewController::onLeftKey(LBool repeated)
{
	
}

void UIViewController::onDownKey(LBool repeated)
{
	HtmlView* item = m_view->getDocument()->getCurrentItem();
	item->setSelected(LFalse);
	//item->setNeedPaint(LTrue);

	item = m_view->getDocument()->getNextItem();
	while (!item->isSelectable())
	{    
	    HtmlView* nextItem = m_view->getDocument()->getNextItem();
	    
	    if (item == nextItem)
	    {
	        break;
	    }
	    
	    item = nextItem;
	}
	
	if (item->isSelectable())
	{
	    item->setSelected(LTrue);

//	    if (item->getYpos() - item->getScrollYPos() > m_view->getClientRange().GetHeight())
//	    {
//	        int scrollYPos = item->getScrollYPos();
//	        scrollYPos += item->getYpos() - scrollYPos - m_view->getClientRange().GetHeight() + m_view->getClientRange().GetHeight()/8;
//	        m_view->getDocument()->getRenderTreeRoot()->setScrollPos(0, scrollYPos);
//	    }
	}
		
	m_view->getLoader()->repaint();
}

void UIViewController::onRightKey(LBool repeated)
{
	
}

void UIViewController::onEnterKey(LBool repeated)
{
    m_view->getDocument()->getCurrentItem()->execute();	
}

void UIViewController::onMousePress(const LPoint& pt)
{
	
}

void UIViewController::onMouseUp(const LPoint& pt)
{
	
}

LBool UIViewController::hitTest(const LPoint& pt)
{
	if (!m_target) return LFalse;
	LayoutPoint topLeft = m_target->getAbsoluteContainerTopLeft();
	LInt x = m_target->getXpos() + topLeft.iX;
	LInt y = m_target->getYpos() + topLeft.iY;
	return pt.iX > x
			&& pt.iX < x + m_target->getWidth()
			&& pt.iY > y
			&& pt.iY < y + m_target->getHeight();
}

void UIViewController::onTouchDown(const LPoint& pt)
{

	//KFORMATLOG("m_target x=%d, y=%d", pt.iX, pt.iY);
	m_target = findViewByPosition(pt, m_view->getDocument()->getRenderTreeRoot());
	m_targetPoint = pt;
	//KFORMATLOG("m_target x=%d, y=%d, w=%d, h=%d", m_target->getXpos(), m_target->getYpos(), m_target->getWidth(), m_target->getHeight());
    if (m_target)
    {
    	LayoutPoint topLeft = m_target->getAbsoluteContainerTopLeft();
    	LInt x = m_target->getXpos() + topLeft.iX;
    	LInt y = m_target->getYpos() + topLeft.iY;
    	KFORMATLOG("m_target x=%d, y=%d, w=%d, h=%d", x, y, m_target->getWidth(), m_target->getHeight());
    	KFORMATLOG("point x=%d, y=%d", pt.iX, pt.iY);

    	if (m_target->getListener())
    	{
    		m_target->getListener()->onPressDown(m_target);
    	}

    }
}

void UIViewController::onTouchUp(const LPoint& pt)
{
    if (hitTest(pt))
	{
    	if (m_target->isSelectable())
    	{
    	    m_target->setSelected(LTrue);
        }

    	if (m_target->getListener())
    	{
    		m_target->getListener()->onPressUp(m_target);
    	}
	}
}

void UIViewController::onTouchMove(const LPoint& pt)
{
	if (m_targetPoint == pt)
	{
		return;
	}
	// Target 只针对第一次touchDown后的元素
	if (m_target)
	{
		// Only Test ScrollY
		LInt deltaY = (pt.iY - m_targetPoint.iY) * 2;
		BlockView* view = m_target->getContainingBlock();

		LInt scrollY = view->getScrollYPos();
		LBool cantScoll = !view->canScroll()
				|| (scrollY == 0 && deltaY >= 0)
				|| (scrollY == view->scrollHeight() && deltaY <= 0);
		if (!cantScoll)
		{
		    LInt newScrollY = scrollY + deltaY;
		    if (newScrollY > 0)
		    {
		    	newScrollY = 0;
		    }
		    else
		    {
		    	newScrollY = -1 * newScrollY > view->scrollHeight() ? -1 * view->scrollHeight() : newScrollY;
		    }

		    m_targetPoint = pt;
		    KFORMATLOG("onTouchMove scrollY=%d newScrollY=%d", scrollY, newScrollY);
			view->setScrollPos(0, newScrollY);
			//UIView::getInstance()->getLoader()->repaint(view);
			//LGraphicsContext* gc = m_view->getGraphicsContext();
			view->paint(*m_view->getGraphicsContext());
		}

		if (m_target->getListener())
	    {
	    	m_target->getListener()->onPressMove(m_target);
	    }
	}
}

}

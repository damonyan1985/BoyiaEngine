/*
 * BlockView.cpp
 *
 *  Created on: 2011-7-1
 *      Author: yanbo
 *   Modify on: 2012-8-03
 *  Description: All Copyright reserved
 */

#include "BlockView.h"
#include "LColor.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "HtmlDocument.h"
#include "FlexLayout.h"

namespace yanbo
{

BlockView::BlockView(
		const String& id,
		LBool selectable)
    : HtmlView(id, LFalse)// block item can't be selected
    , m_isChildrenInline(LTrue)
    , m_isAnonymous(LFalse)
    , m_scrollX(0)
    , m_scrollY(0)
{
}

BlockView::~BlockView()
{
}

void BlockView::setChildrenInline(LBool isInline)
{
	m_isChildrenInline = isInline;
}

LBool BlockView::isChildrenInline()
{
	return m_isChildrenInline;
}

void BlockView::paint(LGraphicsContext& gc)
{
	if (m_style.displayType == util::Style::DISPLAY_NONE)
	{
		return;
	}

	HtmlView::paint(gc);
}

LBool BlockView::isBlockView() const
{
    return m_style.displayType != util::Style::DISPLAY_INLINE;
}

void BlockView::layout()
{
	// style属性优先级最高
	if (m_style.width)
	{
		m_width = m_style.width * m_style.scale;
	}
	else
	{
		m_width = getParent() ? getParent()->getWidth() : m_doc->getViewPort().GetWidth();
	}

	if (m_style.height)
	{
		m_height = m_style.height * m_style.scale;
	}
	else if (HtmlTags::BODY == m_type)
	{
		m_height = m_doc->getViewPort().GetHeight();
	}

    layoutBlock(LFalse);
}

void BlockView::layoutBlock(LBool relayoutChildren)
{
    if (isChildrenInline())
    {
        layoutInlineChildren();
	    return;
    }

    if (m_style.flexDirection == util::Style::FLEX_ROW)
    {
        FlexLayout::flexRowLayout(this);
        KFORMATLOG("layoutInlineBlock, m_height=%d selectable=%d", m_height, isSelectable());
    }
    else if (m_style.flexDirection == util::Style::FLEX_ROW_REVERSE)
    {
    	FlexLayout::flexRowReverse(this);
    }
	else
	{
	    layoutBlockChildren(relayoutChildren);
	}
}

void BlockView::layoutBlockChildren(LBool relayoutChildren)
{
	HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();

	LayoutUnit previousLogicalHeight = 0;
	LayoutUnit absoluteLogicalHeight = 0;
	for (; iter != iterEnd; ++iter)
	{
		HtmlView* child = *iter;
		if (child->isPositioned())
		{
			layoutPositionChild(child, absoluteLogicalHeight);
			continue;
		}

		layoutBlockChild(child, previousLogicalHeight);
	}

	previousLogicalHeight += m_style.topPadding;

	m_height = absoluteLogicalHeight > previousLogicalHeight ? absoluteLogicalHeight : previousLogicalHeight;

	KFORMATLOG("BlockView layoutBlockChildren m_height=%d", m_height);
}

void BlockView::layoutPositionChild(HtmlView* child, LayoutUnit& previousLogicalHeight)
{
	child->setXpos(child->getStyle()->left);
	child->setYpos(child->getStyle()->top);
	child->layout();

	if (previousLogicalHeight < child->getYpos() + child->getHeight())
	{
		previousLogicalHeight = child->getYpos() + child->getHeight();
	}
	KFORMATLOG("adjustPositioned, x=%d, y=%d", child->getStyle()->left, child->getStyle()->top);
	KFORMATLOG("adjustPositioned, w=%d, h=%d", child->getWidth(), child->getHeight());
}

void BlockView::layoutInlineChildren()
{
	RenderContext rc;

	rc.setMaxWidth(m_width);
	rc.setNextLineHeight(0);
	
	LBool center = (getStyle()->positionType == util::Style::STATICPOSITION
				       && getStyle()->textAlignement == LGraphicsContext::TextCenter);
	rc.setCenter(center);
	// add top padding
	rc.addX(getStyle()->leftPadding);
	rc.addY(getStyle()->topPadding);
		
	HtmlViewList::Iterator iter = m_children.begin();
	HtmlViewList::Iterator iterEnd = m_children.end();

	KFORMATLOG("ImageItem Block rc.X=%d", rc.getX());
	for (; iter != iterEnd; ++iter)
	{
	    (*iter)->layout(rc);
	}
	KFORMATLOG("ImageItem Block rc.Y=%d", rc.getY());
	rc.newLine(this);// 换行
	rc.setNextLineHeight(0);

	rc.addY(getStyle()->bottomPadding);

	m_height = rc.getY();
}

void BlockView::layoutBlockChild(HtmlView* child, LayoutUnit& previousLogicalHeight)
{
	int y = previousLogicalHeight + child->getStyle()->topMargin + m_style.topPadding;
	int x = child->getStyle()->leftMargin + m_style.leftPadding;
	child->setXpos(x);
	child->setYpos(y);
	child->layout();
	previousLogicalHeight += child->getHeight();

	KFORMATLOG("BlockView previousLogicalHeight=%d", previousLogicalHeight);
}

void BlockView::addChild(HtmlView* child)
{
	addChild(child, LTrue);
}

void BlockView::addChild(HtmlView* child, LBool isNotAnonymousBlock)
{
	if (isNotAnonymousBlock)
	{
		if (isChildrenInline() && child->isBlockView())
		{
			LInt size = m_children.count();
			HtmlView::addChild(child);
			setChildrenInline(LFalse);
			if (size)
			{
				makeChildrenNonInline(child);
			}
		}
		else if (!isChildrenInline() && child->isInline())
		{
			HtmlViewList::Iterator iter = m_children.end();
		    HtmlView* lastChild = *(--iter);
			if (lastChild && lastChild->isBlockView())
			{
				BlockView* block = static_cast<BlockView*>(lastChild);
				if (block->isAnonymousBlock())
				{
					block->addChild(child, LFalse);
					child->setParent(block);
					return;
				}
			}

			BlockView* b = createAnonymousBlock();
			b->setDocument(getDocument());
			//m_children.push(b);
			HtmlView::addChild(b);
			b->setParent(this);
			b->addChild(child, LFalse);
			child->setParent(b);
		}
		else
		{
			HtmlView::addChild(child);
		}

	}
	else
	{
		HtmlView::addChild(child);
	}
}

void BlockView::makeChildrenNonInline(HtmlView* block)
{
	BlockView* b = createAnonymousBlock();
	b->setDocument(getDocument());
	HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();
	HtmlViewList::Iterator tmpIter;
	for (; iter != iterEnd;)
	{
		tmpIter = iter;
		++iter;
		if (*tmpIter != block)
		{
			b->addChild(*tmpIter, LFalse);
			(*tmpIter)->setParent(b);
			m_children.erase(tmpIter);
		}
		else
		{
			break;
		}
	}

	b->setParent(this);
	insertChild(tmpIter, b);
}

LVoid BlockView::insertChild(HtmlViewList::Iterator& iter, HtmlView* child)
{
	child->setViewIter(m_children.insert(iter, child));
}

BlockView* BlockView::createAnonymousBlock()
{
	BlockView* b = new BlockView(_CS(""), LFalse);
	b->setIsAnonymousBlock(LTrue);
	return b;
}

LBool BlockView::isAnonymousBlock()
{
	return m_isAnonymous;
}

void BlockView::setIsAnonymousBlock(LBool isAnonymous)
{
	m_isAnonymous = isAnonymous;
}

void BlockView::setScrollPos(LInt x, LInt y)
{
	m_scrollX = x;
	m_scrollY = y;
}

LInt BlockView::getScrollXPos() const
{
	return m_scrollX;
}

LInt BlockView::getScrollYPos() const
{
	return m_scrollY;
}

LBool BlockView::canScroll() const
{
    return m_style.height && m_style.height < m_height;
}

LInt BlockView::getHeight() const
{
    return m_style.height ? m_style.height : m_height;
}

LInt BlockView::scrollHeight() const
{
	return m_height - m_style.height;
}
}

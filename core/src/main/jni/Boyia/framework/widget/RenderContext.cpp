/*
 * RenderContext.cpp
 *
 *  Created on: 2011-6-22
 *      Author: yanbo
 */

#include "RenderContext.h"
#include "BlockView.h"

namespace yanbo
{

//  5 pixel
#define NEWLINEBLANK (int)5
//#define ITEMINTERVAL 2

RenderContext::RenderContext()
    : m_maxWidth(0)
    , m_maxHeight(0)
    , m_x(0)
    , m_y(0)
    , m_lineNumber(0)
    , m_center(LFalse)
    , m_nextLineHeight(0)
    , m_itemCountOfline(0)
    , m_newLineXStart(0)
{	
}

RenderContext::~RenderContext()
{	
}

void RenderContext::newLine()
{
	m_x  = m_newLineXStart;
    m_y += m_nextLineHeight + NEWLINEBLANK;
    resetLineItemCount();
}

void RenderContext::newLine(HtmlView* item)
{
	if (m_center == LTrue)
	{
		relayoutLineItems();
	}

    HtmlView* containerItem = item->getContainingBlock();
    if (containerItem)
    {
        m_x  = containerItem->getStyle()->leftPadding;
    }
    else
    {
        m_x  = m_newLineXStart;           
    }
   
    m_y += m_nextLineHeight > 0 ? (m_nextLineHeight + NEWLINEBLANK) : NEWLINEBLANK;
	m_nextLineHeight = 0;
	resetLineItemCount();

    if (m_center == LTrue)
    {
        m_lineItems.clear();
    }
}

LInt RenderContext::getMaxWidth()
{
	return m_maxWidth;
}

void RenderContext::setMaxWidth(LInt maxWidth)
{
	m_maxWidth = maxWidth;
}

LInt RenderContext::getX()
{
    return m_x;
}

void RenderContext::setX(LInt x)
{
    m_x = x;	
}

LInt RenderContext::getY()
{
    return m_y;	
}

void RenderContext::setY(LInt y)
{
    m_y = y;	
}

void RenderContext::addX(LInt x)
{
    m_x += x;	
}

void RenderContext::addY(LInt y)
{
    m_y += y;	
}

LInt RenderContext::getMaxHeight()
{
    return m_maxHeight;	
}

void RenderContext::setMaxHeight(LInt maxHeight)
{
	m_maxHeight = maxHeight;
}

void RenderContext::setNextLineHeight(LInt h)
{
	m_nextLineHeight = m_nextLineHeight < h ? h : m_nextLineHeight;
}

LInt RenderContext::getNextLineHeight()
{
    return m_nextLineHeight;
}

void RenderContext::addItemInterval()
{
	m_x += m_maxWidth/30;
}

void RenderContext::addLineItemCount()
{
	m_itemCountOfline++;
}

void RenderContext::resetLineItemCount()
{
	m_itemCountOfline = 0;
}

LInt RenderContext::getLineItemCount()
{
	return m_itemCountOfline;
}

void RenderContext::setNewLineXStart(LInt x)
{
	m_newLineXStart = x;
}

LInt RenderContext::getNewLineXStart()
{
    return m_newLineXStart;
}

void RenderContext::addLineItem(HtmlView* item)
{
	if (m_center == LTrue)
	{
	    m_lineItems.push(item);
	}
}

void RenderContext::relayoutLineItems()
{
	if (m_lineItems.empty())
	{
		return;
	}

	HtmlViewList::Iterator iter = m_lineItems.begin();
	HtmlViewList::Iterator iterEnd = m_lineItems.end();
	HtmlViewList::Iterator iterFinal = iterEnd;
	iterFinal--;
	int deltaX = (m_maxWidth - ((*iterFinal)->getXpos() + (*iterFinal)->getWidth() - (*iter)->getXpos()))/2;
	//int deltaX = 500;
	KFORMATLOG("RenderContext::relayoutLineItems() deltaX=%d", deltaX);
	KFORMATLOG("RenderContext::relayoutLineItems() m_maxWidth=%d", m_maxWidth);
	KFORMATLOG("RenderContext::relayoutLineItems() (*iterFinal)->getXpos()=%d", (*iterFinal)->getXpos());
	KFORMATLOG("RenderContext::relayoutLineItems() (*iter)->getXpos()=%d", (*iter)->getXpos());
	KFORMATLOG("RenderContext::relayoutLineItems() (*iterFinal)->getWidth()=%d", (*iterFinal)->getWidth());
	//int deltaX = 500;
	for (; iter != iterEnd; ++iter)
	{
		(*iter)->setXpos((*iter)->getXpos() + deltaX);
		KFORMATLOG("RenderContext::relayoutLineItems() TAGNAME=%d", ((int)(*iter)->getTagType()));
		KFORMATLOG("RenderContext::relayoutLineItems() resultX=%d", (*iter)->getXpos());
		KFORMATLOG("RenderContext::relayoutLineItems() getDeltaX=%d", deltaX);
	}
}

void RenderContext::setCenter(LBool center)
{
	m_center = center;
}

LBool RenderContext::getCenter() const
{
	return m_center;
}

}

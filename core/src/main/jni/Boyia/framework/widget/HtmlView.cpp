/*
 * HtmlView.cpp
 *
 *  Created on: 2011-7-1
 *      Author: yanbo
 */

#include "HtmlView.h"
#include "LColor.h"
#include "HtmlDocument.h"
#include "KVector.h"
#include "StringUtils.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "BlockView.h"
#include "KRefPtr.h"

namespace yanbo
{

#define kDefaultInputBorderWidth 2
HtmlView::HtmlView(
		const String& id,
		LBool selectable)
    : m_id(id)
    , m_parent(NULL)
    , m_type(HtmlTags::TAGNONE)
    , m_doc(NULL)
	, m_isViewRoot(LFalse)
    , m_painter(NULL)
    , m_clip(LTrue)
    , m_needLayout(LTrue)
{
	m_style.focusable = selectable;
}

HtmlView::~HtmlView()
{
	HtmlViewList::Iterator iter = m_children.begin();
	HtmlViewList::Iterator endIter = m_children.end();

	for (; iter != endIter; ++iter)
	{
		delete *iter;
	}

	m_children.clear();
}

void HtmlView::layout(RenderContext& rc)
{
    layoutInline(rc);
}

void HtmlView::layoutInline(RenderContext& rc)
{

	if (m_type == HtmlTags::BR)
	{
	    rc.newLine(this);
	}
	else
	{
		rc.addLineItem(this);
		handleXYPos(rc);

		HtmlViewList::Iterator iter = m_children.begin();
		HtmlViewList::Iterator iterEnd = m_children.end();

		for (; iter != iterEnd; ++iter)
		{
			(*iter)->layout(rc);
			m_width += (*iter)->getWidth();
			if (m_height < (*iter)->getHeight())
			{
				m_height = (*iter)->getHeight();
			}
		}

		KFORMATLOG("Inline::layout m_x=%d and width=%d", m_x, m_width);
		rc.addX(m_width);
		rc.setNextLineHeight(m_height);
	}

}

LVoid HtmlView::setPainter(LVoid* painter)
{
    m_painter = painter;
}

LVoid* HtmlView::painter() const
{
	return m_painter;
}

void HtmlView::handleXYPos(RenderContext& rc)
{
    switch (getStyle()->positionType)
    {
    case util::Style::RELATIVEPOSITION:
        {
            m_x = rc.getX() + getStyle()->left;
            m_y = rc.getY() + getStyle()->top;
        }
        break;
    case util::Style::FIXEDPOSITION:
    case util::Style::ABSOLUTEPOSITION:
        {
            m_x = getStyle()->left;
            m_y = getStyle()->top;
        }
        break;
    default:
        {
            m_x = rc.getX();
            m_y = rc.getY();
        }
        break;
    }
    
    rc.setX(m_x);
    rc.setY(m_y);
}

void HtmlView::paint(LGraphicsContext& gc)
{

	gc.setHtmlView(this);

	setClipRect(gc);

	KLOG("HtmlView::paint");
	LayoutPoint topLeft = getAbsoluteContainerTopLeft();
	LayoutUnit x = getXpos();
	LayoutUnit y = getYpos();

	if (getStyle()->positionType != util::Style::FIXEDPOSITION)
	{
		x += topLeft.iX;
		y += topLeft.iY;
	}
	else
	{
		KFORMATLOG("paint, x=%d, y=%d", x, y);
	}

	if (m_type == HtmlTags::BR)
	{
		return;
	}

	//KFORMATLOG("tagName=%s, HtmlView m_bgColor=%x", (const char*)m_tagName.GetBuffer(), getStyle()->bgColor);
	if (!getStyle()->transparent && getStyle()->bgColor.m_alpha != 0)
	{
	    KLOG("not transparent");
	    gc.setBrushStyle(LGraphicsContext::SolidBrush);

	    LRgb bgColor = getStyle()->bgColor;
	    bgColor.m_alpha = bgColor.m_alpha * ((float)getStyle()->drawOpacity/ 255.0f);

	    gc.setBrushColor(bgColor);
		gc.setPenStyle(LGraphicsContext::NullPen);
		gc.drawRect(x, y, m_width, m_height);
	}

	HtmlViewList::Iterator iter    = m_children.begin();
	HtmlViewList::Iterator iterEnd = m_children.end();

	for (; iter != iterEnd; ++iter)
	{
        (*iter)->paint(gc);
	}

	paintBorder(gc, m_style.border, x, y);
}

void HtmlView::paintBorder(LGraphicsContext& gc, const util::Border& border, LayoutUnit x, LayoutUnit y)
{
	if (border.topWidth > 0)
	{
	    gc.setPenStyle((LGraphicsContext::PenStyle)border.topStyle);
	    gc.setPenColor(util::LColor::parseArgbInt(border.topColor));
		gc.setBrushStyle(LGraphicsContext::SolidBrush);
	    gc.setBrushColor(util::LColor::parseArgbInt(border.topColor));
		gc.drawRect(x, y, m_width, border.topWidth);
	}
	else 
	{
		if (m_type == HtmlTags::INPUT)
		{
		    gc.setPenStyle((LGraphicsContext::PenStyle)border.topStyle);
		    gc.setPenColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.setBrushStyle(LGraphicsContext::SolidBrush);
	        gc.setBrushColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
		    gc.drawRect(x, y, m_width, kDefaultInputBorderWidth);
		}

	}
	
	if (border.leftWidth > 0)
	{
	    gc.setPenStyle((LGraphicsContext::PenStyle)border.leftStyle);
	    gc.setPenColor(util::LColor::parseArgbInt(border.leftColor));
		gc.setBrushStyle(LGraphicsContext::SolidBrush);
	    gc.setBrushColor(util::LColor::parseArgbInt(border.leftColor));
	    gc.drawRect(x, y, border.leftWidth, m_height);
	}
	else
	{
		if (m_type == HtmlTags::INPUT)
		{
			gc.setPenStyle((LGraphicsContext::PenStyle)border.topStyle);
			gc.setPenColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.setBrushStyle(LGraphicsContext::SolidBrush);
	        gc.setBrushColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.drawRect(x, y, kDefaultInputBorderWidth, m_height);
		}
	}
	
	if (border.bottomWidth > 0)
	{
	    gc.setPenStyle((LGraphicsContext::PenStyle)border.bottomStyle);
	    gc.setPenColor(util::LColor::parseArgbInt(border.bottomColor));
		gc.setBrushStyle(LGraphicsContext::SolidBrush);
	    gc.setBrushColor(util::LColor::parseArgbInt(border.bottomColor));
	    gc.drawRect(x, y + m_height - border.bottomWidth, m_width, border.bottomWidth);
	}
	else
	{
		if (m_type == HtmlTags::INPUT)
		{
			gc.setPenStyle((LGraphicsContext::PenStyle)border.topStyle);
			gc.setPenColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.setBrushStyle(LGraphicsContext::SolidBrush);
	        gc.setBrushColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.drawRect(x, y + m_height - border.bottomWidth, m_width, kDefaultInputBorderWidth);
		}
	}
	
	if (border.rightWidth > 0)
	{
	    gc.setPenStyle((LGraphicsContext::PenStyle)border.rightStyle);
	    gc.setPenColor(util::LColor::parseArgbInt(border.rightColor));
		gc.setBrushStyle(LGraphicsContext::SolidBrush);
	    gc.setBrushColor(util::LColor::parseArgbInt(border.rightColor));
	    gc.drawRect(x + m_width - border.rightWidth, y, border.rightWidth, m_height);
	}
	else
	{
		if (m_type == HtmlTags::INPUT)
		{
			gc.setPenStyle((LGraphicsContext::PenStyle)border.topStyle);
			gc.setPenColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.setBrushStyle(LGraphicsContext::SolidBrush);
	        gc.setBrushColor(util::LColor::parseRgbInt(COLOR_DARKGRAY));
			gc.drawRect(x + m_width - border.rightWidth, y, kDefaultInputBorderWidth, m_height);
		}
	}
	
}

LBool HtmlView::isText() const
{
	return LFalse;
}

LBool HtmlView::isLink() const
{
	return LFalse;	
}

void HtmlView::execute()
{
}

void HtmlView::setId(const String& id)
{
    m_id = id;	
}

const String& HtmlView::getId() const
{
	return m_id;
}

void HtmlView::setClassName(const String& className)
{
    m_className = className;	
}

const String& HtmlView::getClassName() const
{
	return m_className;
}

void HtmlView::setTagType(HtmlTags::HtmlType tagType)
{
	m_type = tagType;
}

HtmlTags::HtmlType HtmlView::getTagType() const
{
	return m_type;
}

// apply style property for render tree
void HtmlView::setStyle(util::CssManager* manager, util::CssRule* parentRule)
{
	KLOG("HtmlView::setStyle");
	KRefPtr<KVector<String> > classNames = util::StringUtils::split(m_className, _CS(" "));
	String idCss = _CS("#") + m_id;
	int size = classNames->size();
	for (int i=0; i<size; ++i)
	{
	    (*classNames.get())[i] = _CS(".") + (*classNames.get())[i];
	}
	KLOG("HtmlView::setStyle1");
	manager->pushDoctreeNode(idCss, *(classNames.get()), m_tagName);
	KLOG("HtmlView::setStyle2");
	KRefPtr<util::CssRule> newRule = manager->createNewCssRule(parentRule, manager->getCssRule());
	KLOG("HtmlView::setStyle3");
	HtmlViewList::Iterator iter = m_children.begin();
	HtmlViewList::Iterator iterEnd = m_children.end();
	for (; iter != iterEnd; ++iter)
	{  
		(*iter)->setStyle(manager, newRule.get());
	}
    
	manager->popDoctreeNode();
    newRule->createStyle(m_style);
	KLOG("HtmlView::setStyle end");
}

void HtmlView::setSelected(const LBool selected)
{
	m_selected = selected;
}

LBool HtmlView::isSelected()
{
	return m_selected;
}

HtmlView* HtmlView::getPreItem()
{
	HtmlView* item = NULL;
	if(m_parent)
	{
        HtmlViewList::Iterator iter = m_parent->m_children.begin();
        HtmlViewList::Iterator iterEnd = m_parent->m_children.end();
		for (; iter != iterEnd; ++iter)
		{
		    // because klist is a two-way circular linked list, so only iterEnd
		    // can judge whether list is end.
		    HtmlViewList::Iterator tmpIter = iter;
			if (*tmpIter == this && (--tmpIter) != iterEnd)
			{
				if((*iter)->m_children.count())
				{
				    item = getPreItem(*tmpIter);
				}
				else
				{
				    item = *tmpIter;
				}
				
				break;
			}
		}

		if (NULL == item)
		{
		    item = m_parent;
		}
	}

	if(NULL == item)
	{
	    item = this;
	}
	
	return item;
}

HtmlView* HtmlView::getNextItem()
{
	HtmlView* item = NULL;

	if (m_children.count())
	{
	    HtmlViewList::Iterator iter = m_children.begin();
	    item = *iter;
	}
	else
	{
	    item = getNextItem(this);
	}

	if(NULL == item)
	{
	    item = this;
	}

	return item;
}

HtmlView* HtmlView::getPreItem(HtmlView* currentItem)
{
	HtmlView* item = currentItem;
    if (item->m_children.count())
    {
        HtmlViewList::Iterator iter = item->m_children.end();
        item = *(--iter);
    }

    return item;
}

HtmlView* HtmlView::getNextItem(HtmlView* currentItem)
{
	HtmlView* item = NULL;
	if (currentItem->m_parent)
	{
		// 找自己所在的下一个兄弟节点
	    HtmlViewList::Iterator iter = currentItem->m_parent->m_children.begin();
	    HtmlViewList::Iterator iterEnd = currentItem->m_parent->m_children.end();
		for (; iter != iterEnd; ++iter)
		{
			if (*iter == currentItem)
			{
			    if((++iter) != iterEnd)
			    {
					item = *iter;
			    }

			    break;
			}
		}

		if (NULL == item)
		{
		    item = getNextItem(currentItem->m_parent);
	    }
	}

	return item;
}

void HtmlView::itemCenter(RenderContext& rc)
{
	if (getStyle()->textAlignement == util::LGraphicsContext::TextCenter)
	{
	    if(m_x < rc.getNewLineXStart() + rc.getMaxWidth()/2)
	    {
	        m_x = rc.getNewLineXStart() + rc.getMaxWidth()/2;
	    }
	}
}

LBool HtmlView::isSelectable()
{
    return m_style.focusable;
}

LBool HtmlView::isImage() const
{
    return LFalse;	
}

LBool HtmlView::isStyle() const
{
    return LFalse;	
}

LBool HtmlView::isJavaScript() const
{
    return LFalse;	
}

LBool HtmlView::isBlockView() const
{
    return LFalse;	
}

BlockView* HtmlView::getContainingBlock() const
{
	// 根据position的不同，所取的containing block的方式也不一样
	HtmlView* o = getParent();
	if (!isText() && getStyle()->positionType == util::Style::FIXEDPOSITION)
	{
		while (o && !o->isViewRoot() && !(o->hasTransform() && o->isBlockView()))
			o = o->getParent();
	}
	else if (!isText() && getStyle()->positionType == util::Style::ABSOLUTEPOSITION)
	{
		while (o && (o->getStyle()->positionType == util::Style::STATICPOSITION || (o->isInline() && !o->isReplaced())) && !o->isViewRoot() && !(o->hasTransform() && o->isBlockView()))
		{
			if (o->getStyle()->positionType == util::Style::RELATIVEPOSITION && o->isInline() && !o->isReplaced())
			{
				BlockView* relPositionedInlineContainingBlock = o->getContainingBlock();
				while (relPositionedInlineContainingBlock->isAnonymousBlock())
                    relPositionedInlineContainingBlock = relPositionedInlineContainingBlock->getContainingBlock();
                return relPositionedInlineContainingBlock;
			}

			o = o->getParent();
		}
	}
	else 
	{
        while (o && ((o->isInline() && !o->isReplaced()) || !o->isBlockView()))
            o = o->getParent();
    }

    if (!o || !o->isBlockView())
        return NULL; // This can still happen in case of an orphaned tree

	return static_cast<BlockView*>(o);
}

HtmlView* HtmlView::getParent() const
{
	return m_parent;
}

void HtmlView::setParent(HtmlView* o)
{
	m_parent = o;
}

LBool HtmlView::isViewRoot() const
{
	return m_isViewRoot;
}

void HtmlView::setIsViewRoot(LBool isViewRoot)
{
	m_isViewRoot = isViewRoot;
}

void HtmlView::layout()
{
}

void HtmlView::addChild(HtmlView* child)
{
	child->m_iter = m_children.push(child);
}

void HtmlView::removeChild(HtmlView* child)
{
	m_children.erase(child->m_iter);
	delete child;
}

LVoid HtmlView::setViewIter(const HtmlViewList::Iterator& iter)
{
	m_iter = iter;
}

LBool HtmlView::hasTransform() const
{
	return LTrue;
}

LBool HtmlView::isReplaced() const
{
	return LFalse;
}

LBool HtmlView::isInline() const
{
    return LFalse;
}

LBool HtmlView::isPositioned() const
{
	if (getStyle())
	{
		return getStyle()->positionType == util::Style::ABSOLUTEPOSITION || getStyle()->positionType == util::Style::FIXEDPOSITION ? LTrue : LFalse;
	}

	return LFalse;
}

LBool HtmlView::isRelPositioned() const
{
	return getStyle()->positionType == util::Style::RELATIVEPOSITION;
}

util::Style* HtmlView::getStyle() const
{
	return (util::Style*)&m_style;
}

LayoutPoint HtmlView::getAbsoluteContainerTopLeft() const
{
	// 使用getContainingBlock来寻找绝对的topleft坐标有个好处，
	// 就是当position处于不同状态时, 能够取到正确的Container
	BlockView* container = getContainingBlock();
	LayoutUnit top = 0;
	LayoutUnit left = 0;
	while (container && container != this)
	{
		top  += container->getXpos() + container->getScrollXPos();
		left += container->getYpos() + container->getScrollYPos();
		container = container->getContainingBlock();
	}

	return LayoutPoint(top, left);
}

void HtmlView::setDocument(HtmlDocument* doc)
{
    m_doc = doc;
}

HtmlDocument* HtmlView::getDocument() const
{
    return m_doc;
}

LBool HtmlView::isClipItem() const
{
	return m_clip;
}

LayoutRect HtmlView::clipRect() const
{
	return m_clipRect;
}

LVoid HtmlView::setClipRect(util::LGraphicsContext& gc)
{
	HtmlView* parent = getParent();
	if (!parent)
	{
		m_clip = LFalse;
		return;
	}

    //KFORMATLOG("HtmlView::setClipRect X=%d Y=%d pwidth=%d pheight=%d", getXpos(), getYpos(), parent->getWidth(), parent->getHeight());
	if (getXpos() < 0 || getXpos() + getWidth() > parent->getWidth()
			|| getYpos() < 0 || getYpos()  + getHeight() > parent->getHeight())
	{
		LPoint point = parent->getAbsoluteContainerTopLeft();
		LInt parentX = point.iX + parent->getXpos();
		LInt parentY = point.iY + parent->getYpos();

		KFORMATLOG("HtmlView::setClipRect X=%d Y=%d pwidth=%d pheight=%d", parentX, parentY, parent->getWidth(), parent->getHeight());
		m_clipRect = LayoutRect(
				parentX,
				parentY,
				parent->getWidth(),
				parent->getHeight()
		);

		//gc.setClipRect(m_clipRect);
		m_clip = LTrue;
	}
	else
	{
		m_clip = LFalse;
	}

}

LVoid HtmlView::relayoutZIndexChild()
{
	if (!m_children.count())
	{
		return;
	}

	HtmlViewList::Iterator iter = m_children.begin();
	HtmlViewList::Iterator iterEnd = m_children.end();

	HtmlView* lastChild = *(--iterEnd);
	while (iter != iterEnd)
	{
		HtmlView* child = *iter;
		if (child && child->isPositioned() &&
				(child->getStyle()->zindex > lastChild->getStyle()->zindex))
		{
			KFORMATLOG("HtmlView::relayoutZIndexChild child->getStyle()->zindex=%d", child->getStyle()->zindex);
			HtmlViewList::Iterator tmpIter = iter++;
			m_children.erase(tmpIter);
			m_children.push(child);

			lastChild = child;
		}
		else
		{
			++iter;
		}
	}
}

LVoid HtmlView::setListener(IViewListener* listener)
{
	m_itemListener = listener;
}

IViewListener* HtmlView::getListener() const
{
	return m_itemListener;
}

void HtmlView::relayout()
{
    if ((!m_style.width || !m_style.height)
    		&& m_parent)
    {
    	m_parent->relayout();
    }
    else
    {
    	layout();
    }
}

const HtmlViewList& HtmlView::getChildren() const
{
	return m_children;
}

}


/*
 * BlockView.cpp
 *
 *  Created on: 2011-7-1
 *      Author: yanbo
 *   Modify on: 2012-8-03
 *  Description: All Copyright reserved
 */

#include "BlockView.h"
#include "FlexLayout.h"
#include "HtmlDocument.h"
#include "LColor.h"
#include "RenderContext.h"
#include "SalLog.h"

namespace yanbo {

BlockView::BlockView(
    const String& id,
    LBool selectable)
    : HtmlView(id, LFalse) // block item can't be selected
    , m_isChildrenInline(LTrue)
    , m_isAnonymous(LFalse)
    , m_scrollX(0)
    , m_scrollY(0)
{
}

BlockView::~BlockView()
{
}

LVoid BlockView::setChildrenInline(LBool isInline)
{
    m_isChildrenInline = isInline;
}

LBool BlockView::isChildrenInline()
{
    return m_isChildrenInline;
}

LBool BlockView::isBlockView() const
{
    return m_style.displayType != util::Style::DISPLAY_INLINE;
}

LVoid BlockView::layout()
{
    // style属性优先级最高
    if (m_style.width) {
        m_width = m_style.width * m_style.scale;
    } else {
        m_width = getParent() ? (getParent()->getWidth() - m_x) : m_doc->getViewPort().GetWidth();
    }

    if (m_style.height) {
        m_height = m_style.height * m_style.scale;
    } else if (HtmlTags::BODY == m_type) {
        m_height = m_doc->getViewPort().GetHeight();
    } else if (m_style.align == util::Style::ALIGN_ALL) {
        m_height = getParent() ? getParent()->getHeight() : m_doc->getViewPort().GetHeight();
    }

    layoutBlock(LFalse);
}

LVoid BlockView::layoutBlock(LBool relayoutChildren)
{
    if (isChildrenInline()) {
        layoutInlineChildren();
        return;
    }

    if (m_style.flexDirection == util::Style::FLEX_ROW) {
        FlexLayout::flexRowLayout(this);
        BOYIA_LOG("layoutInlineBlock, m_height=%d selectable=%d", m_height, isSelectable());
    } else if (m_style.flexDirection == util::Style::FLEX_ROW_REVERSE) {
        FlexLayout::flexRowReverse(this);
    } else {
        layoutBlockChildren(relayoutChildren);
    }
}

LVoid BlockView::layoutBlockChildren(LBool relayoutChildren)
{
    HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();

    LayoutUnit previousLogicalHeight = 0;
    //LayoutUnit absoluteLogicalHeight = 0;
    for (; iter != iterEnd; ++iter) {
        HtmlView* child = *iter;
        if (child->isPositioned()) {
            layoutPositionChild(child);
            continue;
        }

        layoutBlockChild(child, previousLogicalHeight);
    }

    previousLogicalHeight += m_style.topPadding;

    LBool useLogicHeight = m_style.align != util::Style::ALIGN_ALL
        && !m_style.height;
    if (useLogicHeight && previousLogicalHeight > m_height) {
        m_height = previousLogicalHeight;
    }

    BOYIA_LOG("BlockView layoutBlockChildren m_height=%d", m_height);
}

LVoid BlockView::layoutPositionChild(HtmlView* child)
{
    child->layout();
    if (m_height && child->getStyle()->align == util::Style::ALIGN_BOTTOM) {
        child->setXpos(child->getStyle()->left);
        child->setYpos(m_height - child->getHeight());
    } else if (m_style.width && child->getStyle()->align == util::Style::ALIGN_CENTER) {
        child->setXpos((m_width - child->getWidth()) / 2);
        child->setYpos(child->getStyle()->top);
        BOYIA_LOG("BlockView layoutPositionChild m_width=%d child.width=%d", m_width, child->getWidth());
        //child->setYpos(m_height - child->getHeight());
    } else {
        child->setXpos(child->getStyle()->left);
        child->setYpos(child->getStyle()->top);
    }

    // if (previousLogicalHeight < child->getYpos() + child->getHeight())
    // {
    // 	previousLogicalHeight = child->getYpos() + child->getHeight();
    // }
    BOYIA_LOG("adjustPositioned, x=%d, y=%d", child->getStyle()->left, child->getStyle()->top);
    BOYIA_LOG("adjustPositioned, w=%d, h=%d", child->getWidth(), child->getHeight());
}

LVoid BlockView::layoutInlineChildren()
{
    RenderContext rc;

    rc.setMaxWidth(m_width);
    rc.setNextLineHeight(0);

    LBool center = (getStyle()->positionType == util::Style::STATICPOSITION
        && getStyle()->textAlignement == LGraphicsContext::kTextCenter);
    rc.setCenter(center);
    // add top padding
    rc.addX(getStyle()->leftPadding);
    rc.addY(getStyle()->topPadding);

    HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();

    BOYIA_LOG("ImageItem Block rc.X=%d", rc.getX());
    for (; iter != iterEnd; ++iter) {
        (*iter)->layout(rc);
    }
    BOYIA_LOG("ImageItem Block rc.Y=%d", rc.getY());
    rc.newLine(this); // 换行
    rc.setNextLineHeight(0);

    rc.addY(getStyle()->bottomPadding);

    m_height = rc.getY() > m_height ? rc.getY() : m_height;
}

LVoid BlockView::layoutBlockChild(HtmlView* child, LayoutUnit& previousLogicalHeight)
{
    int y = previousLogicalHeight + child->getStyle()->topMargin + m_style.topPadding;
    int x = child->getStyle()->leftMargin + m_style.leftPadding;
    child->setXpos(x);
    child->setYpos(y);
    child->layout();
    previousLogicalHeight += child->getHeight();

    BOYIA_LOG("BlockView previousLogicalHeight=%d", previousLogicalHeight);
}

LVoid BlockView::addChild(HtmlView* child)
{
    addChild(child, LFalse);
}

LVoid BlockView::addChild(HtmlView* child, LBool isAnonymousBlock)
{
    if (isAnonymousBlock) {
        HtmlView::addChild(child);
        return;
    }

    if (isChildrenInline() && child->isBlockView()) {
        LInt size = m_children.count();
        HtmlView::addChild(child);
        setChildrenInline(LFalse);
        if (size) {
            makeChildrenNonInline(child);
        }
    } else if (!isChildrenInline() && child->isInline()) {
        HtmlViewList::Iterator iter = m_children.end();
        HtmlView* lastChild = *(--iter);
        if (lastChild && lastChild->isBlockView()) {
            BlockView* block = static_cast<BlockView*>(lastChild);
            if (block->isAnonymousBlock()) {
                block->addChild(child, LTrue);
                child->setParent(block);
                return;
            }
        }

        BlockView* b = createAnonymousBlock();
        b->setDocument(getDocument());
        HtmlView::addChild(b);
        b->setParent(this);
        b->addChild(child, LTrue);
        child->setParent(b);
    } else {
        HtmlView::addChild(child);
    }
}

LVoid BlockView::makeChildrenNonInline(HtmlView* block)
{
    BlockView* b = createAnonymousBlock();
    b->setDocument(getDocument());
    HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();
    HtmlViewList::Iterator tmpIter;
    for (; iter != iterEnd;) {
        tmpIter = iter;
        ++iter;
        if (*tmpIter != block) {
            b->addChild(*tmpIter, LTrue);
            (*tmpIter)->setParent(b);
            m_children.erase(tmpIter);
        } else {
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

LVoid BlockView::setIsAnonymousBlock(LBool isAnonymous)
{
    m_isAnonymous = isAnonymous;
}

LVoid BlockView::setScrollPos(LInt x, LInt y)
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
    if (getParent()) {
        return m_height > getParent()->getHeight();
    }

    return m_height > m_doc->getViewPort().GetHeight();
}

LInt BlockView::getHeight() const
{
    return m_style.height ? m_style.height : m_height;
}

LInt BlockView::scrollHeight() const
{
    if (getParent()) {
        return m_height - getParent()->getHeight();
    }

    return m_height - m_doc->getViewPort().GetHeight();
}
}

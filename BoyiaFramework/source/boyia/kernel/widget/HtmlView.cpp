/*
 * HtmlView.cpp
 *
 *  Created on: 2011-7-1
 *      Author: yanbo
 */

#include "HtmlView.h"
#include "BlockView.h"
#include "BoyiaPtr.h"
#include "HtmlDocument.h"
#include "KVector.h"
#include "LColorUtil.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "UIView.h"
#include "PixelRatio.h"

namespace yanbo {
HtmlView::HtmlView(const String& id, LBool selectable)
    : m_id(id)
    , m_parent(kBoyiaNull)
    , m_type(HtmlTags::TAGNONE)
    , m_doc(kBoyiaNull)
    , m_isViewRoot(LFalse)
    , m_clip(LTrue)
    , m_needLayout(LTrue)
    , m_itemListener(kBoyiaNull)
    , m_selected(LFalse)
    , m_viewId(0)
{
    m_style.focusable = selectable;
}

HtmlView::~HtmlView()
{
}

LVoid HtmlView::layout(RenderContext& rc)
{
    layoutInline(rc);
}

LVoid HtmlView::layoutInline(RenderContext& rc)
{
    if (m_type == HtmlTags::BR) {
        rc.newLine(this);
    } else {
        rc.addLineItem(this);
        handleXYPos(rc);

        HtmlViewList::Iterator iter = m_children.begin();
        HtmlViewList::Iterator iterEnd = m_children.end();

        for (; iter != iterEnd; ++iter) {
            (*iter)->layout(rc);
            m_width += (*iter)->getWidth();
            if (m_height < (*iter)->getHeight()) {
                m_height = (*iter)->getHeight();
            }
        }

        BOYIA_LOG("Inline::layout m_x=%d and width=%d", m_x, m_width);
        rc.addX(m_width);
        rc.setNextLineHeight(m_height);
    }
}

LVoid HtmlView::handleXYPos(RenderContext& rc)
{
    switch (getStyle()->positionType) {
    case util::Style::RELATIVEPOSITION: {
        m_x = rc.getX() + getStyle()->left;
        m_y = rc.getY() + getStyle()->top;
    } break;
    case util::Style::FIXEDPOSITION:
    case util::Style::ABSOLUTEPOSITION: {
        m_x = getStyle()->left;
        m_y = getStyle()->top;
    } break;
    default: {
        m_x = rc.getX();
        m_y = rc.getY();
    } break;
    }

    rc.setX(m_x);
    rc.setY(m_y);
}

LVoid HtmlView::paint(LGraphicsContext& gc)
{
    gc.setHtmlView(this);
    
    if (m_style.displayType == util::Style::DISPLAY_NONE) {
        return;
    }
    
    if (!canDraw()) {
        return;
    }
    
    setClipRect(gc);

    KLOG("HtmlView::paint");
    LayoutPoint topLeft = getAbsoluteContainerTopLeft();
    LayoutUnit x = getXpos();
    LayoutUnit y = getYpos();

    if (getStyle()->positionType != util::Style::FIXEDPOSITION) {
        x += topLeft.iX;
        y += topLeft.iY;
    } else {
        BOYIA_LOG("paint, x=%d, y=%d", x, y);
    }

    if (m_type == HtmlTags::BR) {
        gc.restore();
        return;
    }

    //KFORMATLOG("tagName=%s, HtmlView m_bgColor=%x", (const char*)m_tagName.GetBuffer(), getStyle()->bgColor);
    if (!getStyle()->transparent && getStyle()->bgColor.m_alpha != 0) {
        KLOG("not transparent");
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);

        LColor bgColor = getStyle()->bgColor;
        bgColor.m_alpha = bgColor.m_alpha * ((float)getStyle()->drawOpacity / 255.0f);

        gc.setBrushColor(bgColor);
        gc.setPenStyle(LGraphicsContext::kNullPen);
        
        if (getStyle()->radius().topLeftRadius
            || getStyle()->radius().topRightRadius
            || getStyle()->radius().bottomLeftRadius
            || getStyle()->radius().bottomRightRadius) {
            gc.drawRoundRect(LRect(x, y, m_width, m_height),
                             getStyle()->radius().topLeftRadius,
                             getStyle()->radius().topRightRadius,
                             getStyle()->radius().bottomRightRadius,
                             getStyle()->radius().bottomLeftRadius);
        } else {
            gc.drawRect(x, y, m_width, m_height);
        }
        
    }

    HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();

    for (; iter != iterEnd; ++iter) {
        (*iter)->paint(gc);
    }
    
    paintBorder(gc, m_style.border(), x, y);

    gc.restore();
}

LVoid HtmlView::paintBorder(LGraphicsContext& gc, const util::Border& border, LayoutUnit x, LayoutUnit y)
{
    if (border.topWidth > 0) {
        gc.setPenStyle((LGraphicsContext::PenStyle)border.topStyle);
        gc.setPenColor(util::LColorUtil::parseArgbInt(border.topColor));
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(util::LColorUtil::parseArgbInt(border.topColor));
        gc.drawRect(x, y, m_width, border.topWidth);
    }

    if (border.leftWidth > 0) {
        gc.setPenStyle((LGraphicsContext::PenStyle)border.leftStyle);
        gc.setPenColor(util::LColorUtil::parseArgbInt(border.leftColor));
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(util::LColorUtil::parseArgbInt(border.leftColor));
        gc.drawRect(x, y, border.leftWidth, m_height);
    }

    if (border.bottomWidth > 0) {
        gc.setPenStyle((LGraphicsContext::PenStyle)border.bottomStyle);
        gc.setPenColor(util::LColorUtil::parseArgbInt(border.bottomColor));
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(util::LColorUtil::parseArgbInt(border.bottomColor));
        gc.drawRect(x, y + m_height - border.bottomWidth, m_width, border.bottomWidth);
    }

    if (border.rightWidth > 0) {
        gc.setPenStyle((LGraphicsContext::PenStyle)border.rightStyle);
        gc.setPenColor(util::LColorUtil::parseArgbInt(border.rightColor));
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(util::LColorUtil::parseArgbInt(border.rightColor));
        gc.drawRect(x + m_width - border.rightWidth, y, border.rightWidth, m_height);
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

LVoid HtmlView::execute()
{
}

LVoid HtmlView::setId(const String& id)
{
    m_id = id;
}

const String& HtmlView::getId() const
{
    return m_id;
}

LVoid HtmlView::setClassName(const String& className)
{
    m_className = className;
}

const String& HtmlView::getClassName() const
{
    return m_className;
}

LVoid HtmlView::setTagType(HtmlTags::HtmlType tagType)
{
    m_type = tagType;
}

HtmlTags::HtmlType HtmlView::getTagType() const
{
    return m_type;
}

// apply style property for render tree
LVoid HtmlView::setStyle(util::StyleManager* manager, util::StyleRule* parentRule)
{
    KLOG("HtmlView::setStyle");
    BoyiaPtr<KVector<String>> classNames = util::StringUtils::split(m_className, _CS(" "));
    String idCss = _CS("#") + m_id;
    int size = classNames->size();
    for (int i = 0; i < size; ++i) {
        (*classNames.get())[i] = _CS(".") + (*classNames.get())[i];
    }
    KLOG("HtmlView::setStyle1");
    manager->pushDoctreeNode(idCss, *(classNames.get()), m_tagName);
    KLOG("HtmlView::setStyle2");
    BoyiaPtr<util::StyleRule> newRule = manager->createNewStyleRule(parentRule, manager->getStyleRule());
    KLOG("HtmlView::setStyle3");
    HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();
    for (; iter != iterEnd; ++iter) {
        (*iter)->setStyle(manager, newRule.get());
    }

    manager->popDoctreeNode();
    newRule->createStyle(m_style);
    KLOG("HtmlView::setStyle end");
}

LVoid HtmlView::setSelected(const LBool selected)
{
    m_selected = selected;
}

LBool HtmlView::isSelected()
{
    return m_selected;
}

HtmlView* HtmlView::getPreItem()
{
    HtmlView* item = kBoyiaNull;
    if (m_parent) {
        HtmlViewList::Iterator iter = m_parent->m_children.begin();
        HtmlViewList::Iterator iterEnd = m_parent->m_children.end();
        for (; iter != iterEnd; ++iter) {
            // because klist is a two-way circular linked list, so only iterEnd
            // can judge whether list is end.
            HtmlViewList::Iterator tmpIter = iter;
            if (*tmpIter == this && (--tmpIter) != iterEnd) {
                if ((*iter)->m_children.count()) {
                    item = getPreItem(*tmpIter);
                } else {
                    item = *tmpIter;
                }

                break;
            }
        }

        if (!item) {
            item = m_parent;
        }
    }

    if (!item) {
        item = this;
    }

    return item;
}

HtmlView* HtmlView::getNextItem()
{
    HtmlView* item = kBoyiaNull;

    if (m_children.count()) {
        HtmlViewList::Iterator iter = m_children.begin();
        item = *iter;
    } else {
        item = getNextItem(this);
    }

    if (!item) {
        item = this;
    }

    return item;
}

HtmlView* HtmlView::getPreItem(HtmlView* currentItem)
{
    HtmlView* item = currentItem;
    if (item->m_children.count()) {
        HtmlViewList::Iterator iter = item->m_children.end();
        item = *(--iter);
    }

    return item;
}

HtmlView* HtmlView::getNextItem(HtmlView* currentItem)
{
    HtmlView* item = kBoyiaNull;
    if (currentItem->m_parent) {
        // 找自己所在的下一个兄弟节点
        HtmlViewList::Iterator iter = currentItem->m_parent->m_children.begin();
        HtmlViewList::Iterator iterEnd = currentItem->m_parent->m_children.end();
        for (; iter != iterEnd; ++iter) {
            if (*iter == currentItem) {
                if ((++iter) != iterEnd) {
                    item = *iter;
                }

                break;
            }
        }

        if (!item) {
            item = getNextItem(currentItem->m_parent);
        }
    }

    return item;
}

LVoid HtmlView::itemCenter(RenderContext& rc)
{
    if (getStyle()->textAlignement == util::LGraphicsContext::kTextCenter) {
        if (m_x < rc.getNewLineXStart() + rc.getMaxWidth() / 2) {
            m_x = rc.getNewLineXStart() + rc.getMaxWidth() / 2;
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

LBool HtmlView::isBlockView() const
{
    return LFalse;
}

BlockView* HtmlView::getContainingBlock() const
{
    // 根据position的不同，所取的containing block的方式也不一样
    HtmlView* o = getParent();
    if (!isText() && getStyle()->positionType == util::Style::FIXEDPOSITION) {
        while (o && !o->isViewRoot() && !(o->hasTransform() && o->isBlockView()))
            o = o->getParent();
    } else if (!isText() && getStyle()->positionType == util::Style::ABSOLUTEPOSITION) {
        while (o && (o->getStyle()->positionType == util::Style::STATICPOSITION || (o->isInline() && !o->isReplaced())) && !o->isViewRoot() && !(o->hasTransform() && o->isBlockView())) {
            if (o->getStyle()->positionType == util::Style::RELATIVEPOSITION && o->isInline() && !o->isReplaced()) {
                BlockView* relPositionedInlineContainingBlock = o->getContainingBlock();
                while (relPositionedInlineContainingBlock->isAnonymousBlock())
                    relPositionedInlineContainingBlock = relPositionedInlineContainingBlock->getContainingBlock();
                return relPositionedInlineContainingBlock;
            }

            o = o->getParent();
        }
    } else {
        while (o && ((o->isInline() && !o->isReplaced()) || !o->isBlockView()))
            o = o->getParent();
    }

    if (!o || !o->isBlockView())
        return kBoyiaNull; // This can still happen in case of an orphaned tree

    return static_cast<BlockView*>(o);
}

HtmlView* HtmlView::getParent() const
{
    return m_parent;
}

LVoid HtmlView::setParent(HtmlView* o)
{
    m_parent = o;
}

LBool HtmlView::isViewRoot() const
{
    return m_isViewRoot;
}

LVoid HtmlView::setIsViewRoot(LBool isViewRoot)
{
    m_isViewRoot = isViewRoot;
}

LVoid HtmlView::layout()
{
}

LVoid HtmlView::addChild(HtmlView* child)
{
    child->m_iter = m_children.push(child);
}

LVoid HtmlView::removeChild(HtmlView* child)
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
    if (getStyle()) {
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

LBool HtmlView::canDraw() const
{
    LayoutPoint point = getAbsoluteTopLeft();
    //LayoutPoint point = getAbsoluteContainerTopLeft();
    // 目前仅对超出屏幕的元素进行了判断
    if (PixelRatio::isInWindow(LRect(point.iX, point.iY, m_width, m_height))) {
        return LTrue;
    }
    
    return LFalse;
}

LayoutPoint HtmlView::getAbsoluteContainerTopLeft() const
{
    // 使用getContainingBlock来寻找绝对的topleft坐标有个好处，
    // 就是当position处于不同状态时, 能够取到正确的Container
    BlockView* container = getContainingBlock();
    LayoutUnit top = 0;
    LayoutUnit left = 0;
    while (container && container != this) {
        left += container->getXpos() + container->getScrollXPos();
        top += container->getYpos() + container->getScrollYPos();
        container = container->getContainingBlock();
    }

    return LayoutPoint(left, top);
}

LVoid HtmlView::setDocument(HtmlDocument* doc)
{
    m_doc = doc;
    // View Id必须与UIView进行绑定
    m_viewId = doc->getView()->incrementViewId();
}

HtmlDocument* HtmlView::getDocument() const
{
    return m_doc;
}

LayoutRect HtmlView::clipRect() const
{
    return m_clipRect;
}

LayoutPoint HtmlView::getAbsoluteTopLeft() const
{
    HtmlView* parent = getParent();
    if (!parent) {
        return LayoutPoint(0, 0);
    }

    LayoutPoint point = getAbsoluteContainerTopLeft();
    LayoutUnit left = (parent->isBlockView() ? 0 : parent->getXpos()) + point.iX + getXpos();
    LayoutUnit top = (parent->isBlockView() ? 0 : parent->getYpos()) + point.iY + getYpos();
    return LayoutPoint(left, top);
}

LVoid HtmlView::setClipRect(LGraphicsContext& gc)
{
    HtmlView* parent = getParent();
    if (!parent) {
        m_clipRect = LRect(m_x, m_y, m_width, m_height);
        return;
    }

    //KFORMATLOG("HtmlView::setClipRect X=%d Y=%d pwidth=%d pheight=%d", getXpos(), getYpos(), parent->getWidth(), parent->getHeight());
    // 获取绝对坐标
    LPoint point = getAbsoluteTopLeft();
    LayoutUnit left = point.iX;
    LayoutUnit top = point.iY;
    LayoutUnit right = left + getWidth();
    LayoutUnit bottom = top + getHeight();

    LRect clipRect = parent->clipRect();
    
    if (left < clipRect.iTopLeft.iX || right > clipRect.iBottomRight.iX
        || top < clipRect.iTopLeft.iY || bottom > clipRect.iBottomRight.iY) {

        //BOYIA_LOG("HtmlView::setClipRect X=%d Y=%d pwidth=%d pheight=%d", parentX, parentY, parent->getWidth(), parent->getHeight());

        LInt clipL = left < clipRect.iTopLeft.iX ? clipRect.iTopLeft.iX : left;
        LInt clipT = top < clipRect.iTopLeft.iY ? clipRect.iTopLeft.iY : top;

        LInt clipR = right > clipRect.iBottomRight.iX ? clipRect.iBottomRight.iX : right;
        LInt clipB = bottom > clipRect.iBottomRight.iY ? clipRect.iBottomRight.iY : bottom;

        gc.clipRect(clipRect);
        m_clipRect = LayoutRect(
            clipL,
            clipT,
            clipR - clipL,
            clipB - clipT);
    } else {
        m_clipRect = LRect(
            left, 
            top, 
            m_width, 
            m_height);
        gc.restore();
    }
}

LVoid HtmlView::relayoutZIndexChild()
{
    if (!m_children.count()) {
        return;
    }

    HtmlViewList::Iterator iter = m_children.begin();
    HtmlViewList::Iterator iterEnd = m_children.end();

    HtmlView* lastChild = *(--iterEnd);
    while (iter != iterEnd) {
        HtmlView* child = *iter;
        if (child && child->isPositioned() && (child->getStyle()->zindex > lastChild->getStyle()->zindex)) {
            KFORMATLOG("HtmlView::relayoutZIndexChild child->getStyle()->zindex=%d", child->getStyle()->zindex);
            HtmlViewList::Iterator tmpIter = iter++;
            m_children.erase(tmpIter);
            m_children.push(child);

            lastChild = child;
        } else {
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

LVoid HtmlView::relayout()
{
    if ((!m_style.width || !m_style.height)
        && m_parent) {
        m_parent->relayout();
    } else {
        layout();
    }
}

const HtmlViewList& HtmlView::getChildren() const
{
    return m_children;
}

LInt HtmlView::viewId() const
{
    return m_viewId;
}

// 根据标签类型判断显示状态
LVoid HtmlView::setVisible(LBool visible)
{
    if (isInline()) {
        m_style.displayType = visible ? util::Style::DISPLAY_INLINE : util::Style::DISPLAY_NONE;
    } else if (isBlockView()) {
        LBaseView::setVisible(visible);
    }
}
}

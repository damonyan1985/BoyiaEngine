/*
 * TextView.cpp
 *
 *  Created on: 2011-7-12
 *      Author: yanbo
 */

#include "TextView.h"
#include "StringUtils.h"
//#include "SalLog.h"
#include "LColorUtil.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "PlatformBridge.h"

namespace yanbo {
const LInt kDefaultMaxLine = 10;

class TextSelection {
public:
    TextSelection()
        : start(0)
        , end(0)
    {
    }
  
public:
    LInt start;
    LInt end;
};

class TextLine : public ViewPainter {
public:
    TextLine(TextView* view, LInt length, const String& text)
        : m_view(view)
        , m_lineLength(length)
        , m_text(text)
    {
    }

    ~TextLine()
    {
    }
    
    virtual LBool canDraw() const
    {
        //return LTrue;
//        const LRect& rect = m_view->getParent()->clipRect();
//        if (point.iX + getWidth() < clipRect.iTopLeft.iX
//            || point.iY + getHeight() < clipRect.iTopLeft.iY
//            || point.iX > clipRect.iBottomRight.iX
//            || point.iY > clipRect.iBottomRight.iY) {
//            return LFalse;
//        }
        
        return LTrue;
    }

    LVoid paint(LGraphicsContext& gc, const LRect& rect)
    {
        if (!canDraw()) {
            return;
        }
        m_rect = rect;
        gc.setHtmlView(this);
        gc.drawText(m_text, rect, LGraphicsContext::kTextLeft);
    }

public:
    TextView* m_view;
    LInt m_lineLength;
    String m_text;
    LRect m_rect;
};

TextView::TextView(const String& text)
    : InlineView(_CS(""), LFalse)
    , m_text(text)
    , m_textLines(0, kDefaultMaxLine)
    , m_newFont(kBoyiaNull)
    , m_maxWidth(0)
{
}

TextView::~TextView()
{
}

ViewPainter* TextView::linePainter(LInt i) const
{
    return m_textLines.elementAt(i);
}

LInt TextView::lineSize() const
{
    return m_textLines.size();
}

LInt TextView::lineWidth(LInt i) const
{
    if (i >= m_textLines.size()) {
        return 0;
    }
    return m_textLines.elementAt(i)->m_lineLength;
}

LInt TextView::lineLength(LInt i) const
{
    if (i >= m_textLines.size()) {
        return 0;
    }
    
    return PlatformBridge::getTextSize(m_textLines.elementAt(i)->m_text);
}

LInt TextView::getIndexByOffset(LInt line, LInt x)
{
    if (!m_newFont) {
        return 0;
    }
    
    return m_newFont->getIndexByOffset(line, x);
}

LInt TextView::getOffsetByIndex(LInt line, LInt index)
{
    if (!m_newFont || index == 0) {
        return 0;
    }
    
    return m_newFont->getOffsetByIndex(line, index);
}

void TextView::layout(RenderContext& rc)
{
    handleXYPos(rc);

    m_width = 0;
    m_height = 0;

    m_textLines.clear();

    m_maxWidth = rc.getMaxWidth() - m_x;

    KSTRFORMAT("TextView::layout m_text=%s", m_text);

    m_newFont = LFont::create(m_style.font); // platform reference font
    m_newFont->setFontSize(m_style.font.getFontSize() * m_style.scale);
    m_width = calcTextLine(m_text, m_maxWidth);

    //m_width = longestLine + m_style.margin().leftMargin + m_style.margin().rightMargin;
    
    BOYIA_LOG("text=%s and text width=%d", (const char*)m_text.GetBuffer(), m_width);
    //m_height = m_newFont->getFontHeight() * m_textLines->size() + m_style.margin().bottomMargin;
    m_height = m_newFont->getFontHeight() * m_textLines.size();
    
    KLOG("TextView::layout begin");
    KDESLOG(m_height);
    KDESLOG(m_newFont->getFontHeight());
    //KDESLOG(m_style.margin().bottomMargin);
    KDESLOG(getBottomY());
    //rc.setX(getEndX());
    //rc.setY(getBottomY() - m_newFont->getFontHeight());
    //rc.setNextLineHeight(m_newFont->getFontHeight());
    //rc.addItemInterval();
    KLOG("TextView::layout end");

    if (getParent()->isBlockView()) {
        rc.addLineItem(this);
        rc.addX(m_width);
        rc.setNextLineHeight(m_height);
    }
}

void TextView::setText(const String& text)
{
    m_text = text;
}

void TextView::setAlignement(LGraphicsContext::TextAlign alignement)
{
    m_style.textAlignement = alignement;
}

// Calculate the text lines
LInt TextView::calcTextLine(const String& text, LInt maxWidth)
{
    LInt longestLine = m_newFont->calcTextLine(text, maxWidth);
    LInt len = m_newFont->getLineSize();

    BOYIA_LOG("TextView calcTextLine text=%s and len=%d longestLine=%d maxWidth=%d fontSize=%d", (const char*)m_text.GetBuffer(), len, longestLine, maxWidth, m_newFont->getFontSize());
    for (LInt i = 0; i < len; ++i) {
        String text;
        m_newFont->getLineText(i, text);
        m_textLines.addElement(new TextLine(
            this,
            m_newFont->getLineWidth(i),
            text));
    }

    return longestLine;
}

LBool TextView::isMultiLine()
{
    return m_textLines.size() > 1 ? LTrue : LFalse;
}

LVoid TextView::paint(LGraphicsContext& gc)
{
    //gc.setHtmlView(this);
    setClipRect(gc);
    //LayoutPoint topLeft = getAbsoluteContainerTopLeft();
    LayoutPoint topLeft = getAbsoluteTopLeft();
    BOYIA_LOG("TextView::paint topLeft.x=%d", topLeft.iX);
    //LayoutUnit x = (getParent()->isBlockView() ? 0 : getParent()->getXpos()) + topLeft.iX + getXpos();
    //LayoutUnit y = (getParent()->isBlockView() ? 0 : getParent()->getYpos()) + topLeft.iY + getYpos();
    LayoutUnit x = topLeft.iX;
    LayoutUnit y = topLeft.iY;

    if (m_newFont) {
        LInt textHeight = m_newFont->getFontHeight();
        LInt len = m_textLines.size();
        for (LInt i = 0; i < len; ++i) {
            TextLine* line = m_textLines.elementAt(i);

            y = i > 0 ? y + textHeight : y;
            LayoutUnit left = x;
            if (m_style.textAlignement == LGraphicsContext::kTextCenter) { // 居中对齐
                left += (m_width - line->m_lineLength) / 2;

                BOYIA_LOG("TextView::paint center left=%d", line->m_lineLength);
            } else if (m_style.textAlignement == LGraphicsContext::kTextRight) { // 右对齐
                left += m_width - line->m_lineLength;
            }

            if (!line->m_lineLength || !textHeight) {
                return;
            }

            KLOG("draw Text");

            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setFont(*m_newFont);

            LColor color = m_style.color;
            color.m_alpha = color.m_alpha * ((LReal)m_style.drawOpacity / 255.0f);
            
            gc.setPenColor(color);
            BOYIA_LOG("text length=%d", line->m_lineLength);
            BOYIA_LOG("text x=%d, y=%d", x, y);
            gc.setPenStyle(LGraphicsContext::kSolidPen);

            gc.setBrushColor(m_style.bgColor);

            line->paint(gc, LRect(left, y, line->m_lineLength, textHeight));
        }
    }

    
    gc.restore();
}

LBool TextView::isText() const
{
    return LTrue;
}

HtmlView* TextView::getLinkItem()
{
    HtmlView* parent = getParent();
    KSTRFORMAT("TextView parent tagname=%s", parent->getTagName());
    while (parent && !parent->isLink()) {
        parent = parent->getParent();
    }

    return parent;
}
}

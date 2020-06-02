/*
 * TextView.cpp
 *
 *  Created on: 2011-7-12
 *      Author: yanbo
 */

#include "TextView.h"
#include "StringUtils.h"
//#include "SalLog.h"
#include "LColor.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "StringUtils.h"

namespace yanbo {
class TextLine : public ViewPainter {
public:
    TextLine(LInt length, const String& text)
    {
        m_lineLength = length;
        m_text = text;
    }

    ~TextLine()
    {
    }

    LVoid paint(LGraphicsContext& gc, const LRect& rect)
    {
        gc.setHtmlView(this);
        gc.drawText(m_text, rect, LGraphicsContext::kTextLeft);
    }

public:
    LInt m_lineLength;
    String m_text;
};

TextView::TextView(const String& id, const String& text)
    : InlineView(id, LFalse)
    , m_text(text)
    , m_textLines(kBoyiaNull)
    , m_newFont(kBoyiaNull)
    , m_maxWidth(0)
{
}

TextView::~TextView()
{
    if (m_newFont) {
        delete m_newFont;
    }

    if (m_textLines) {
        delete m_textLines;
    }
}

ViewPainter* TextView::linePainter(LInt i) const
{
    return m_textLines->elementAt(i);
}

LInt TextView::lineSize() const
{
    return m_textLines->size();
}

void TextView::layout(RenderContext& rc)
{
    handleXYPos(rc);

    m_width = 0;
    m_height = 0;

    if (!m_textLines) {
        m_textLines = new KVector<TextLine*>();
    } else {
        m_textLines->clear();
    }

    m_maxWidth = rc.getMaxWidth() - m_x;

    KSTRFORMAT("TextView::layout m_text=%s", m_text);

    if (m_newFont) {
        delete m_newFont;
    }

    LFont font = m_style.font;
    font.setFontSize(font.getFontSize() * m_style.scale);

    m_newFont = LFont::create(font); // platform reference font
    LInt longestLine = calcTextLine(m_text, m_maxWidth - m_style.leftMargin - m_style.rightMargin);

    m_width = longestLine + m_style.leftMargin + m_style.rightMargin;
    KFORMATLOG("text=%s and text width=%d and lineWidth=%d", (const char*)m_text.GetBuffer(), m_width, longestLine);
    m_height = m_newFont->getFontHeight() * m_textLines->size() + m_style.bottomMargin;

    KLOG("TextView::layout begin");
    KDESLOG(m_height);
    KDESLOG(m_newFont->getFontHeight());
    KDESLOG(m_style.bottomMargin);
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
    } else {
        m_x = m_x - getParent()->getXpos();
        m_y = m_y - getParent()->getYpos();
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

    KFORMATLOG("TextView calcTextLine text=%s and len=%d longestLine=%d maxWidth=%d fontSize=%d", (const char*)m_text.GetBuffer(), len, longestLine, maxWidth, m_newFont->getFontSize());
    for (LInt i = 0; i < len; ++i) {
        String text;
        m_newFont->getLineText(i, text);
        m_textLines->addElement(new TextLine(
            m_newFont->getLineWidth(i),
            text));
    }

    return longestLine;
}

LBool TextView::isMultiLine()
{
    return m_textLines->size() > 1 ? LTrue : LFalse;
}

LVoid TextView::paint(LGraphicsContext& gc)
{
    //gc.setHtmlView(this);
    setClipRect(gc);
    LayoutPoint topLeft = getAbsoluteContainerTopLeft();
    KFORMATLOG("TextView::paint topLeft.x=%d", topLeft.iX);
    LayoutUnit x = (getParent()->isBlockView() ? 0 : getParent()->getXpos()) + topLeft.iX + getXpos();
    LayoutUnit y = (getParent()->isBlockView() ? 0 : getParent()->getYpos()) + topLeft.iY + getYpos();

    if (m_newFont) {
        LInt textHeight = m_newFont->getFontHeight();
        LInt len = m_textLines->size();
        for (LInt i = 0; i < len; ++i) {
            TextLine* line = m_textLines->elementAt(i);

            y = i > 0 ? y + textHeight : y;
            LayoutUnit left = x;
            if (m_style.textAlignement == LGraphicsContext::kTextCenter) { // 居中对齐
                left += (m_width - line->m_lineLength) / 2;

                KFORMATLOG("TextView::paint center left=%d", line->m_lineLength);
            } else if (m_style.textAlignement == LGraphicsContext::kTextRight) { // 右对齐
                left += m_width - line->m_lineLength;
            }

            if (!line->m_lineLength || !textHeight) {
                return;
            }

            KLOG("draw Text");

            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setFont(*m_newFont);

            LRgb color = m_style.color;
            color.m_alpha = color.m_alpha * ((float)m_style.drawOpacity / 255.0f);

            gc.setPenColor(color);
            KFORMATLOG("text length=%d", line->m_lineLength);
            KFORMATLOG("text x=%d, y=%d", x, y);
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

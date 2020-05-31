/*
 * InputView.cpp
 *
 *  Created on: 2011-7-17
 *      Author: Administrator
 */

#include "InputView.h"
#include "LColor.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "PlatformBridge.h"

namespace yanbo {

const LInt kDefaultInputBorderWidth = 2;

InputView::InputView(
    const String& id,
    const String& name,
    const String& value,
    const String& title,
    const String& imageUrl)
    : FormView(id, name, value, title)
    , m_newFont(kBoyiaNull)
{
    m_value = value;
    m_title = title;

    initView();
}

LVoid InputView::initView()
{
    switch (getInputType()) {
    case kInputText:
    case kInputPassword:
    case kInputFile:
        m_style.bgColor.m_alpha = 0;
        break;
    case kInputSubmit:
    case kInputButton:
    case kInputReset:
        m_style.bgColor = util::LColor::parseArgbInt(COLOR_LIGHTGRAY);
        break;
    }
    m_style.border.topColor = COLOR_BLACK;
    m_style.border.leftColor = COLOR_BLACK;
    m_style.border.rightColor = COLOR_BLACK;
    m_style.border.bottomColor = COLOR_BLACK;
    m_style.border.topWidth = kDefaultInputBorderWidth;
    m_style.border.leftWidth = kDefaultInputBorderWidth;
    m_style.border.rightWidth = kDefaultInputBorderWidth;
    m_style.border.bottomWidth = kDefaultInputBorderWidth;
    m_style.border.topStyle = LGraphicsContext::kSolidPen;
    m_style.border.leftStyle = LGraphicsContext::kSolidPen;
    m_style.border.rightStyle = LGraphicsContext::kSolidPen;
    m_style.border.bottomStyle = LGraphicsContext::kSolidPen;
}

InputView::~InputView()
{
    if (m_newFont) {
        delete m_newFont;
    }

    Editor::get()->removeView(this);
}

LVoid InputView::setInputValue(const String& text)
{
    m_value = text;
}

LVoid InputView::layoutBegin(RenderContext& rc)
{
    rc.addLineItem(this);
    if (m_newFont) {
        delete m_newFont;
    }

    m_newFont = LFont::create(getStyle()->font);
}

LVoid InputView::layoutEnd(RenderContext& rc)
{
    if (kInputHidden != m_type && rc.getX() >= 0) {
        if (rc.getX() + m_leftPadding + m_width > rc.getMaxWidth() + rc.getNewLineXStart()) {
            rc.newLine(this);
            rc.setNextLineHeight(m_height);
        } else {
            if (rc.getLineItemCount() >= 0 && m_height > rc.getNextLineHeight()) {
                rc.setNextLineHeight(m_height);
            }
        }
    }

    setPos(rc.getX() + rc.getMaxWidth() / 80, rc.getY());

    //itemCenter(rc);

    KLOG("InputView::layout");
    KDESLOG(m_y);
    KDESLOG(m_height);

    rc.addX(m_width);

    if (m_type != kInputHidden) {
        rc.addItemInterval();
        rc.addLineItemCount();
    }
}

LVoid InputView::paintBegin(LGraphicsContext& gc, LayoutPoint& point)
{
    gc.setHtmlView(this);
    setClipRect(gc);

    LayoutPoint topLeft = getAbsoluteContainerTopLeft();
    LInt x = topLeft.iX + getXpos();
    LInt y = topLeft.iY + getYpos();

    if (!m_newFont) {
        m_newFont = LFont::create(getStyle()->font);
    }
    gc.setFont(*m_newFont);
    LRgb color = m_style.color;
    color.m_alpha = color.m_alpha * ((float)m_style.drawOpacity / 255.0f);
    gc.setPenColor(color);

    point.iX = x;
    point.iY = y;
}

LVoid InputView::execute()
{
}

LInt InputView::getInputType()
{
    return kInputText;
}

class InputTextBox : public InputView {
public:
    InputTextBox(
        const String& id,
        const String& name,
        const String& value,
        const String& title,
        const String& imageUrl)
        : InputView(id, name, value, title, imageUrl)
    {
    }

    LVoid layout(RenderContext& rc)
    {
        InputView::layoutBegin(rc);
        LInt maxWidth = rc.getMaxWidth();
        m_leftPadding = 5;
        m_width = getStyle()->width ? getStyle()->width : maxWidth / 3;
        m_height = getStyle()->height ? getStyle()->height : m_newFont->getFontHeight() + 6;
        InputView::layoutEnd(rc);
    }

    virtual LInt getInputType() 
    {
        return kInputText;
    }

    virtual LVoid paint(LGraphicsContext& gc)
    {
        LayoutPoint point;
        paintTextBorder(gc, point);
        if (m_value.GetLength() == 0) {
            return;
        }
        gc.drawText(m_value, 
            LRect(point.iX + m_leftPadding, 
                point.iY + 6, m_width - m_leftPadding, 
                m_height - 6), 
            util::LGraphicsContext::kTextLeft);
    }

    LVoid paintTextBorder(LGraphicsContext& gc, LayoutPoint& point)
    {
        paintBegin(gc, point);
        if (m_style.bgColor.m_alpha) {
            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setBrushColor(getStyle()->bgColor);

            gc.setPenStyle(LGraphicsContext::kNullPen);
            gc.drawRect(point.iX + m_leftPadding, point.iY, m_width, m_height);
        }

        paintBorder(gc, getStyle()->border, point.iX + m_leftPadding, point.iY);
    }

    virtual LVoid setSelected(const LBool selected)
    {
        HtmlView::setSelected(selected);
        Editor::get()->setView(this)->showKeyboard(m_value);
    }
};

class InputPassword : public InputTextBox {
public:
    InputPassword(
        const String& id,
        const String& name,
        const String& value,
        const String& title,
        const String& imageUrl)
        : InputTextBox(id, name, value, title, imageUrl)
    {
    }

    virtual LInt getInputType()
    {
        return kInputPassword;
    }

    virtual LVoid paint(LGraphicsContext& gc)
    {
        LayoutPoint point;
        paintTextBorder(gc, point);
        if (m_value.GetLength() == 0) {
            return;
        }
        
        gc.drawText(
            m_value, 
            LRect(point.iX + m_leftPadding, point.iY + 6, 
                m_width - m_leftPadding, m_height - 6), util::LGraphicsContext::kTextLeft);
    }
};

class InputButton : public InputView {
public:
    InputButton(
        const String& id,
        const String& name,
        const String& value,
        const String& title,
        const String& imageUrl)
        : InputView(id, name, value, title, imageUrl)
    {
    }

    virtual LVoid layout(RenderContext& rc)
    {
        InputView::layoutBegin(rc);
        LInt maxWidth = rc.getMaxWidth();
        m_leftPadding = 0;
        m_width = maxWidth / 5;
        m_height = m_newFont->getFontHeight() + 10;
        InputView::layoutEnd(rc);
    }

    virtual LInt getInputType()
    {
        return kInputButton;
    }

    virtual LVoid paint(LGraphicsContext& gc)
    {
        LayoutPoint point;
        InputView::paintBegin(gc, point);

        if (getStyle()->bgColor.m_alpha == 0) {
            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setBrushColor(util::LColor::parseArgbInt(COLOR_LIGHTGRAY));

            gc.setPenStyle(LGraphicsContext::kSolidPen);
            gc.setPenColor(util::LColor::parseArgbInt(COLOR_DARKGRAY));
            gc.drawRect(point.iX + m_leftPadding, point.iY, m_width, m_height);
        } else {
            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setBrushColor(getStyle()->bgColor);

            gc.setPenStyle(LGraphicsContext::kNullPen);
            gc.drawRect(point.iX + m_leftPadding, point.iY, m_width, m_height);

            gc.setPenStyle(LGraphicsContext::kSolidPen);
        }

        paintBorder(gc, getStyle()->border, point.iX, point.iY);

        if (m_value.GetLength() > 0) {
            gc.setPenStyle(LGraphicsContext::kSolidPen);
            gc.setPenColor(getStyle()->color);

            gc.drawText(m_value,
                LRect(point.iX + m_leftPadding,
                    point.iY + 6,
                    m_width - m_leftPadding,
                    m_height - 6),
                LGraphicsContext::kTextCenter);
        }
    }
};

InputView* InputView::create(
    const String& id,
    const String& name,
    const String& value,
    const String& title,
    const String& inputType,
    const String& imageUrl)
{
    if (inputType.CompareNoCase(_CS("text"))) {
        return new InputTextBox(id, name, value, title, imageUrl);
    } else if (inputType.CompareNoCase(_CS("password"))) {
        return new InputPassword(id, name, value, title, imageUrl);
    } else if (inputType.CompareNoCase(_CS("reset"))) {
    } else if (inputType.CompareNoCase(_CS("radio"))) {
    } else if (inputType.CompareNoCase(_CS("checkbox"))) {
    } else if (inputType.CompareNoCase(_CS("button"))) {
        return new InputButton(id, name, value, title, imageUrl);
    } else if (inputType.CompareNoCase(_CS("submit"))) {
        return new InputButton(id, name, value, title, imageUrl);
    }

    return kBoyiaNull;
}
}

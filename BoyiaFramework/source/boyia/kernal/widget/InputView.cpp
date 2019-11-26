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

namespace yanbo {

const LInt kDefaultInputBorderWidth = 2;

InputView::InputView(
    const String& id,
    const String& name,
    const String& value,
    const String& title,
    const String& inputType,
    const String& imageUrl)
    : FormView(id, name, value, title)
    , m_newFont(kBoyiaNull)
{
    m_type = kInputText;
    m_value = value;
    m_title = title;

    if (inputType.CompareNoCase(_CS("text"))) {
        m_type = kInputText;
    } else if (inputType.CompareNoCase(_CS("password"))) {
        m_type = kInputPassword;
    } else if (inputType.CompareNoCase(_CS("reset"))) {
        m_type = kInputReset;
    } else if (inputType.CompareNoCase(_CS("radio"))) {
        m_type = kInputRadio;
    } else if (inputType.CompareNoCase(_CS("checkbox"))) {
        m_type = kInputCheckbox;
    } else if (inputType.CompareNoCase(_CS("file"))) {
        m_type = kInputFile;
    } else if (inputType.CompareNoCase(_CS("hidden"))) {
        m_type = kInputHidden;
    } else if (inputType.CompareNoCase(_CS("button"))) {
        m_type = kInputButton;
    } else if (inputType.CompareNoCase(_CS("submit"))) {
        m_type = kInputSubmit;
    } else if (inputType.CompareNoCase(_CS("image"))) {
        m_type = kInputImage;
    } else {
        m_type = kInputNone;
    }

    initView();
}

LVoid InputView::initView()
{
    switch (m_type) {
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

LVoid InputView::layout(RenderContext& rc)
{
    rc.addLineItem(this);
    int maxWidth = rc.getMaxWidth();
    if (m_newFont) {
        delete m_newFont;
    }

    m_newFont = LFont::create(getStyle()->font);
    switch (m_type) {
    case kInputButton:
    case kInputSubmit:
    case kInputReset: {
        //m_leftPadding = maxWidth / 80;
        m_leftPadding = 0;
        //m_width  = m_width + 6;
        m_width = maxWidth / 5;
        m_height = m_newFont->getFontHeight() + 10;
    } break;
    case kInputText:
    case kInputPassword: {
        m_leftPadding = 5;
        m_width = getStyle()->width ? getStyle()->width : maxWidth / 3;
        m_height = getStyle()->height ? getStyle()->height : m_newFont->getFontHeight() + 6;
    } break;
    case kInputImage: {

    } break;
    case kInputHidden: {
        m_width = 0;
        m_height = 0;
        m_leftPadding = 0;
    } break;
    case kInputFile: {
        m_width = maxWidth / 2;
        m_leftPadding = maxWidth / 80;
        m_height = m_newFont->getFontHeight() + 6;
        m_mimeType = _CS("text/plain"); //test
        m_value = _CS("File upload not supported");
    } break;
    case kInputCheckbox: {
        m_leftPadding = maxWidth / 25;
        m_width = m_height = m_newFont->getFontHeight();
    } break;
    case kInputRadio: {
        m_leftPadding = maxWidth / 25;
        m_width = m_height = m_newFont->getFontHeight();
    } break;
    }

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

LVoid InputView::paint(LGraphicsContext& gc)
{
    if (m_type == kInputNone) {
        return;
    }

    gc.setHtmlView(this);
    setClipRect(gc);

    LayoutPoint topLeft = getAbsoluteContainerTopLeft();
    LInt x = topLeft.iX + getXpos();
    LInt y = topLeft.iY + getYpos();

    if (kInputHidden == m_type) {
        return;
    }

    if (!m_newFont) {
        m_newFont = LFont::create(getStyle()->font);
    }
    gc.setFont(*m_newFont);
    LRgb color = m_style.color;
    color.m_alpha = color.m_alpha * ((float)m_style.drawOpacity / 255.0f);
    gc.setPenColor(color);

    switch (m_type) {
    case kInputText:
    case kInputPassword:
    case kInputFile: {
        paintTextBox(gc, x, y);
    } break;
    case kInputSubmit:
    case kInputButton:
    case kInputReset: {
        paintButton(gc, x, y);
    } break;
    case kInputCheckbox: {

    } break;
    }
}

LVoid InputView::paintTextBox(LGraphicsContext& gc, LayoutUnit x, LayoutUnit y)
{
    if (m_style.bgColor.m_alpha) {
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(getStyle()->bgColor);

        gc.setPenStyle(LGraphicsContext::kNullPen);
        gc.drawRect(x + m_leftPadding, y, m_width, m_height);
    }

    paintBorder(gc, getStyle()->border, x + m_leftPadding, y);

    if (m_value.GetLength() == 0) {
        return;
    }

    if (m_type == kInputPassword) {
        gc.drawText(
            String('*', m_value.GetLength()),
            LRect(x + m_leftPadding, y + 6,
                m_width - m_leftPadding, m_height - 6),
            util::LGraphicsContext::kTextLeft);
    } else {
        gc.drawText(m_value, LRect(x + m_leftPadding, y + 6, m_width - m_leftPadding, m_height - 6), util::LGraphicsContext::kTextLeft);
    }
}

LVoid InputView::paintButton(LGraphicsContext& gc, LayoutUnit x, LayoutUnit y)
{
    if (getStyle()->bgColor.m_alpha == 0) {
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(util::LColor::parseArgbInt(COLOR_LIGHTGRAY));

        gc.setPenStyle(LGraphicsContext::kSolidPen);
        gc.setPenColor(util::LColor::parseArgbInt(COLOR_DARKGRAY));
        gc.drawRect(x + m_leftPadding, y, m_width, m_height);
    } else {
        gc.setBrushStyle(LGraphicsContext::kSolidBrush);
        gc.setBrushColor(getStyle()->bgColor);

        gc.setPenStyle(LGraphicsContext::kNullPen);
        gc.drawRect(x + m_leftPadding, y, m_width, m_height);

        gc.setPenStyle(LGraphicsContext::kSolidPen);
    }

    paintBorder(gc, getStyle()->border, x, y);

    if (m_value.GetLength() > 0) {
        gc.setPenStyle(LGraphicsContext::kSolidPen);
        gc.setPenColor(getStyle()->color);
        //util::String strW;
        //util::StringUtils::strTostrW(m_value, strW);
        //dc.drawText(strW, util::LPoint(m_x + m_leftPadding, m_y - m_scrollY + m_height));
        gc.drawText(m_value,
            LRect(x + m_leftPadding,
                y + 6,
                m_width - m_leftPadding,
                m_height - 6),
            LGraphicsContext::kTextCenter);
    }
}

LVoid InputView::paintRadioButton(LGraphicsContext& dc, LayoutUnit x, LayoutUnit y)
{
}

LVoid InputView::paintCheckBox(LGraphicsContext& dc, LayoutUnit x, LayoutUnit y)
{
}

LVoid InputView::setSelected(const LBool selected)
{
    HtmlView::setSelected(selected);
    if (selected) {
        switch (m_type) {
        case kInputText:
        case kInputPassword: {
            Editor::get()->setView(this)->showKeyboard(m_value);
        } break;
        }
    }
}

LVoid InputView::execute()
{
}

LInt InputView::getInputType()
{
    return m_type;
}
}

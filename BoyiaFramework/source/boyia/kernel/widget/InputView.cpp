/*
 * InputView.cpp
 *
 *  Created on: 2011-7-17
 *      Author: yanbo
 */

#include "InputView.h"
#include "LColorUtil.h"
#include "PlatformBridge.h"
#include "RenderContext.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "Animation.h"
#include "UIThread.h"
#include "UIView.h"

namespace yanbo {
const LInt kDefaultInputBorderWidth = 2;
const LInt kDefaultInputTextPadding = 5;
const LInt kDefaultInputButtonHeight = 20;
const LInt kDefaultInputCursorBlinkTime = 600; // 毫秒

// text input cursor
class TextInputCursor {
public:
    TextInputCursor(LInt cursorHeight, HtmlView* view)
        : m_cursorHeight(cursorHeight)
        , m_view(view)
        , m_paint(LFalse)
        , m_timer(kBoyiaNull)
        , m_cursorIndex(0)
    {
    }
    
    LBool isBlink()
    {
        return m_timer != kBoyiaNull;
    }
    
    LVoid cancel()
    {
        if (m_timer) {
            m_timer->quit();
        }
        
        m_timer = kBoyiaNull;
    }
    
    LVoid startBlink()
    {
        cancel();
        
        m_timer = new Timer(kDefaultInputCursorBlinkTime, [self = this]() -> LVoid {
            self->m_paint = self->m_timer == kBoyiaNull ? LFalse : !self->m_paint;
            //Editor* editor = self->m_view->getDocument()->getEditor();
            UIThread::instance()->draw(self->m_view);
        }, LTrue);
    }
    
    LInt getHeight() const
    {
        return m_cursorHeight;
    }
 
    // point is left top point
    LVoid paint(LGraphicsContext& gc, LayoutPoint point)
    {
        if (!m_paint) {
            return;
        }
        gc.setBrushColor(LColorUtil::parseArgbInt(COLOR_BLACK));
        gc.drawRect(point.iX + kDefaultInputBorderWidth, point.iY, kDefaultInputBorderWidth, m_cursorHeight);
    }
    
    LVoid setTextIndexCursor(const LInt index)
    {
        m_cursorIndex = index;
    }
    
    LInt cursorIndex() const
    {
        return m_cursorIndex;
    }
    
    ~TextInputCursor()
    {
        cancel();
    }
    
private:
    LInt m_cursorHeight;
    Timer* m_timer;
    LBool m_paint;
    LInt m_cursorIndex;
    HtmlView* m_view;
};

InputView::InputView(
    const String& id,
    const String& name,
    const String& value,
    const String& title,
    const String& imageUrl)
    : FormView(id, name, value, title)
{
    m_value = value;
    m_title = title;

    initView();
    
    m_text = new TextView(value);
    addChild(m_text);
    m_text->setParent(this);
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
        m_style.bgColor = util::LColorUtil::parseArgbInt(COLOR_LIGHTGRAY);
        break;
    }
    m_style.border().topColor = COLOR_BLACK;
    m_style.border().leftColor = COLOR_BLACK;
    m_style.border().rightColor = COLOR_BLACK;
    m_style.border().bottomColor = COLOR_BLACK;
    m_style.border().topWidth = kDefaultInputBorderWidth;
    m_style.border().leftWidth = kDefaultInputBorderWidth;
    m_style.border().rightWidth = kDefaultInputBorderWidth;
    m_style.border().bottomWidth = kDefaultInputBorderWidth;
    m_style.border().topStyle = LGraphicsContext::kSolidPen;
    m_style.border().leftStyle = LGraphicsContext::kSolidPen;
    m_style.border().rightStyle = LGraphicsContext::kSolidPen;
    m_style.border().bottomStyle = LGraphicsContext::kSolidPen;
}

InputView::~InputView()
{
}

LVoid InputView::setInputValue(const String& text)
{
    m_value = text;
    m_text->setText(text);
    layoutText();
}

LVoid InputView::setInputCursor(const LInt cursor) {}

LVoid InputView::layoutBegin(RenderContext& rc)
{
    rc.addLineItem(this);
}

LVoid InputView::layoutText()
{
    RenderContext rc;

    rc.setMaxWidth(m_width);
    rc.setNextLineHeight(0);
    m_text->layout(rc);
    
    // always layout in y center
    LInt yPos = (m_height - m_text->getHeight()) / 2;
    m_text->setYpos(yPos);
    m_text->setXpos(kDefaultInputTextPadding);
}

LVoid InputView::layoutEnd(RenderContext& rc)
{
    if (kInputHidden != getInputType() && rc.getX() >= 0) {
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

    KLOG("InputView::layout");
    KDESLOG(m_y);
    KDESLOG(m_height);

    rc.addX(m_width);

    if (getInputType() != kInputHidden) {
        rc.addItemInterval();
        rc.addLineItemCount();
    }
    
    layoutText();
}

LVoid InputView::paintBegin(LGraphicsContext& gc, LayoutPoint& point)
{
    gc.setHtmlView(this);
    setClipRect(gc);

    LayoutPoint topLeft = getAbsoluteContainerTopLeft();
    LInt x = topLeft.iX + getXpos();
    LInt y = topLeft.iY + getYpos();

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

LVoid InputView::setSelectedWithPosition(const LBool selected, const LayoutPoint& point)
{
    HtmlView::setSelected(selected);
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
        , m_cursor(kBoyiaNull)
    {
    }
    
    virtual ~InputTextBox()
    {
        if (m_cursor) {
            delete m_cursor;
        }
        
        getDocument()->getEditor()->removeView(this);
    }

    LVoid layout(RenderContext& rc)
    {
        InputView::layoutBegin(rc);
        LInt maxWidth = rc.getMaxWidth();
        m_leftPadding = 5;
        m_width = getStyle()->width ? getStyle()->width : maxWidth / 3;
        m_height = getStyle()->height ? getStyle()->height : kDefaultInputButtonHeight;
        InputView::layoutEnd(rc);
        
        m_cursor = new TextInputCursor(m_height * 0.75, this);
    }

    virtual LInt getInputType()
    {
        return kInputText;
    }

    virtual LVoid paint(LGraphicsContext& gc)
    {
        LayoutPoint point;
        paintTextBorder(gc, point);
        
        LInt cursorY = point.iY + (m_height - m_cursor->getHeight())/2;
        
        if (m_value.GetLength() == 0) {
            m_cursor->paint(gc, LayoutPoint(point.iX, cursorY));
            return;
        }
        
//        gc.setBrushColor(LColor(0xff, 0x00, 0xFF));
//        gc.drawRect(clipRect());
        //gc.clipRect(LRect(m_x, m_y, m_width, m_height));
        
        BOYIA_LOG("boyia cursor: x=%d c_x=%d y=%d c_y=%d",
                  point.iX,
                  point.iX + m_text->lineWidth(0),
                  point.iY,
                  point.iY + ((m_height - m_cursor->getHeight())/2));
        
//        LayoutPoint cursorPosition = m_cursor->position();
//        LInt textWidth = m_text->lineWidth(0);
//        LInt textSize = m_text->lineLength(0);
//        LInt perWith = textWidth/textSize;
//        LInt cursorDelta = cursorPosition.iX - point.iX;
//        LInt index = cursorDelta/perWith > textSize ? textSize : cursorDelta/perWith;
//
//        LInt cursorX = cursorDelta > 0 ? index * perWith : 0;
        
        LInt cursorX = m_text->getOffsetByIndex(0, m_cursor->cursorIndex());
        
        m_cursor->paint(gc, LayoutPoint(point.iX + cursorX, cursorY));

        m_text->paint(gc);
    }

    LVoid paintTextBorder(LGraphicsContext& gc, LayoutPoint& point)
    {
        paintBegin(gc, point);
        // 绘制背景
        if (m_style.bgColor.m_alpha) {
            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setBrushColor(getStyle()->bgColor);

            gc.setPenStyle(LGraphicsContext::kNullPen);
            if (getStyle()->hasRadius()) {
                gc.drawRoundRect(LRect(point.iX, point.iY, m_width, m_height),
                                 getStyle()->radius().topLeftRadius,
                                 getStyle()->radius().topRightRadius,
                                 getStyle()->radius().bottomRightRadius,
                                 getStyle()->radius().bottomLeftRadius);
            } else {
                gc.drawRect(point.iX, point.iY, m_width, m_height);
            }
        }
        
        // 绘制边界
        paintBorder(gc, getStyle()->border(), point.iX, point.iY);
        
        gc.restore();
    }
    
    virtual LBool isEditor() const
    {
        return LTrue;
    }
    
    LVoid setInputCursor(const LInt cursor)
    {
        if (m_cursor) {
            m_cursor->setTextIndexCursor(cursor);
        }
    }
    
    
    virtual LVoid setSelectedWithPosition(const LBool selected, const LayoutPoint& point)
    {
        HtmlView::setSelected(selected);
        if (selected) {
            if (!m_cursor) {
                return;
            }
            // TODO 必须是已经layout过了
            LayoutPoint topLeft = getAbsoluteContainerTopLeft();
            LInt x = topLeft.iX + getXpos();
            //LInt y = topLeft.iY + getYpos();
            
            LInt deltaX = point.iX - x;
            // 通过点击位置判断焦点是在哪个文字后面
            LInt index = m_text->getIndexByOffset(0, deltaX);
            
            m_cursor->setTextIndexCursor(index);
            
            getDocument()->getEditor()->setView(this)->showKeyboard(m_value, index);
            if (!m_cursor->isBlink()) {
                m_cursor->startBlink();
            }
        } else {
            if (m_cursor) {
                m_cursor->cancel();
            }
        }
    }
    
private:
    TextInputCursor* m_cursor;
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
    
    virtual LVoid setInputValue(const String& text)
    {
        m_value = text;
        m_text->setText(String('*', PlatformBridge::getTextSize(m_value)));
        layoutText();
    }

//    virtual LVoid paint(LGraphicsContext& gc)
//    {
//        LayoutPoint point;
//        paintTextBorder(gc, point);
//        if (m_value.GetLength() == 0) {
//            return;
//        }
//        
//        m_text->paint(gc);
//    }
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
        m_width = m_style.width ? m_style.width : maxWidth / 5;
        m_height = m_style.height ? m_style.width : kDefaultInputButtonHeight;
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
            gc.setBrushColor(util::LColorUtil::parseArgbInt(COLOR_LIGHTGRAY));

            gc.setPenStyle(LGraphicsContext::kSolidPen);
            gc.setPenColor(util::LColorUtil::parseArgbInt(COLOR_DARKGRAY));
            gc.drawRect(point.iX + m_leftPadding, point.iY, m_width, m_height);
        } else {
            gc.setBrushStyle(LGraphicsContext::kSolidBrush);
            gc.setBrushColor(getStyle()->bgColor);

            gc.setPenStyle(LGraphicsContext::kNullPen);
            gc.drawRect(point.iX + m_leftPadding, point.iY, m_width, m_height);

            gc.setPenStyle(LGraphicsContext::kSolidPen);
        }

        paintBorder(gc, getStyle()->border(), point.iX, point.iY);

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

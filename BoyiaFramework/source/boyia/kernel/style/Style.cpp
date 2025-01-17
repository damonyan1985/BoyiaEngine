/*
 *  Style.cpp
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */

#include "Style.h"

#include "LColorUtil.h"

namespace util {

Border::Border()
    : topColor(0)
    , leftColor(0)
    , rightColor(0)
    , bottomColor(0)
    , topWidth(0)
    , leftWidth(0)
    , rightWidth(0)
    , bottomWidth(0)
{
    topStyle = LGraphicsContext::kSolidPen;
    leftStyle = LGraphicsContext::kSolidPen;
    rightStyle = LGraphicsContext::kSolidPen;
    bottomStyle = LGraphicsContext::kSolidPen;
}

Border::~Border()
{
}

LVoid Border::init()
{
    topColor = 0;
    leftColor = 0;
    rightColor = 0;
    bottomColor = 0;
    topWidth = 0;
    leftWidth = 0;
    rightWidth = 0;
    bottomWidth = 0;
    topStyle = LGraphicsContext::kSolidPen;
    leftStyle = LGraphicsContext::kSolidPen;
    rightStyle = LGraphicsContext::kSolidPen;
    bottomStyle = LGraphicsContext::kSolidPen;
}

Border::Border(const Border& border)
{
    copy(border);
}

const Border& Border::operator=(const Border& border)
{
    copy(border);
    return *this;
}

LVoid Border::copy(const Border& border)
{
    topColor = border.topColor;
    leftColor = border.leftColor;
    rightColor = border.rightColor;
    bottomColor = border.bottomColor;
    topWidth = border.topWidth;
    leftWidth = border.leftWidth;
    rightWidth = border.rightWidth;
    bottomWidth = border.bottomWidth;
    topStyle = border.topStyle;
    leftStyle = border.leftStyle;
    rightStyle = border.rightStyle;
    bottomStyle = border.bottomStyle;
}

BorderRadius::BorderRadius()
    : topLeftRadius(0)
    , topRightRadius(0)
    , bottomLeftRadius(0)
    , bottomRightRadius(0)
{
}

BorderRadius::BorderRadius(const BorderRadius& radius)
    : topLeftRadius(radius.topLeftRadius)
    , topRightRadius(radius.topRightRadius)
    , bottomLeftRadius(radius.bottomLeftRadius)
    , bottomRightRadius(radius.bottomRightRadius)
{
}

LVoid BorderRadius::copy(const BorderRadius& radius)
{
    topLeftRadius = radius.topLeftRadius;
    topRightRadius = radius.topRightRadius;
    bottomLeftRadius = radius.bottomLeftRadius;
    bottomRightRadius = radius.bottomRightRadius;
}

Margin::Margin()
    : topMargin(0)
    , bottomMargin(0)
    , leftMargin(0)
    , rightMargin(0)
{
}

LVoid Margin::copy(const Margin& margin)
{
    topMargin = margin.topMargin;
    bottomMargin = margin.bottomMargin;
    leftMargin = margin.leftMargin;
    rightMargin = margin.rightMargin;
}

Padding::Padding()
    : topPadding(0)
    , bottomPadding(0)
    , leftPadding(0)
    , rightPadding(0)
{
}

LVoid Padding::copy(const Padding &padding)
{
    topPadding = padding.topPadding;
    bottomPadding = padding.bottomPadding;
    leftPadding = padding.leftPadding;
    rightPadding = padding.rightPadding;
}

Flex::Flex()
    : flexGrow(0)
{
}

LVoid Flex::copy(const Flex& flex)
{
    flexGrow = flex.flexGrow;
}

Style::Style()
{
    init();
    //border.init();
}

Style::Style(LFont::FontStyle style)
{
    font.setFontStyle(style);
    init();
    //border.init();
}

Style::Style(const Style& style)
{
    color = style.color;
    bgColor = style.bgColor;
    opacity = style.opacity;
    drawOpacity = style.drawOpacity;
    font = style.font;
    positionType = style.positionType;
    left = style.left;
    top = style.top;
    width = style.width;
    height = style.height;
    transparent = style.transparent;
    textAlignement = style.textAlignement;
    displayType = style.displayType;
    scale = style.scale;
    focusable = style.focusable;
    flexDirection = style.flexDirection;
    align = style.align;

    //border = style.getBorder();
    border().copy(style.border());
    radius().copy(style.radius());
    margin().copy(style.margin());
    padding().copy(style.padding());
    flex().copy(style.flex());
}

const Style& Style::operator=(const Style& style)
{
    color = style.color;
    bgColor = style.bgColor;
    opacity = style.opacity;
    drawOpacity = style.drawOpacity;
    font = style.font;
    positionType = style.positionType;
    left = style.left;
    top = style.top;
    width = style.width;
    height = style.height;
    transparent = style.transparent;
    textAlignement = style.textAlignement;
    displayType = style.displayType;
    focusable = style.focusable;
    flexDirection = style.flexDirection;
    
    align = style.align;
    border().copy(style.border());
    radius().copy(style.radius());
    margin().copy(style.margin());
    padding().copy(style.padding());
    flex().copy(style.flex());
    return *this;
}

void Style::init()
{
    color = LColor(0x0, 0x0, 0x0, 0xFF);
    bgColor = LColor(0xFF, 0xFF, 0xFF, 0xFF);
    opacity = 255;
    drawOpacity = 255;
    positionType = STATICPOSITION;
    displayType = DISPLAY_ANY;
    left = 0;
    top = 0;
    width = 0;
    height = 0;
    
    transparent = LTrue;
    textAlignement = LGraphicsContext::kTextLeft;
    focusable = LFalse;
    scale = 1;
    flexDirection = FLEX_NONE;
    align = ALIGN_NONE;
    
    m_border = kBoyiaNull;
    m_radius = kBoyiaNull;
    m_margin = kBoyiaNull;
    m_padding = kBoyiaNull;
    m_flex = kBoyiaNull;
}

Border& Style::border() const
{
    if (!m_border) {
        m_border = new Border();
    }

    return *m_border;
}

BorderRadius& Style::radius() const
{
    if (!m_radius) {
        m_radius = new BorderRadius();
    }
    
    return *m_radius;
}

Margin& Style::margin() const
{
    if (!m_margin) {
        m_margin = new Margin();
    }
    
    return *m_margin;
}

Padding& Style::padding() const
{
    if (!m_padding) {
        m_padding = new Padding();
    }
    
    return *m_padding;;
}

LBool Style::hasRadius() const
{
    return radius().topLeftRadius > 0
        || radius().topRightRadius > 0
        || radius().bottomLeftRadius > 0
        || radius().bottomRightRadius > 0;
}

Flex& Style::flex() const
{
    if (!m_flex) {
        m_flex = new Flex();
    }
    
    return *m_flex;
}

}

/*
 * LBaseView.cpp
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#include "LBaseView.h"
#include "SalLog.h"

namespace util {

LBaseView::LBaseView()
    : m_visible(LTrue)
    , m_x(0)
    , m_y(0)
    , m_width(0)
    , m_height(0)
    , m_leftPadding(0)
    , m_topPadding(0)
{
}

LBaseView::~LBaseView()
{
}

void LBaseView::setTransparent(LBool transparent)
{
    m_style.transparent = transparent;
}

LBool LBaseView::getTransparency()
{
    return m_style.transparent;
}

void LBaseView::setFont(LFont font)
{
    m_style.font = font;
}

const LFont& LBaseView::getFont() const
{
    return m_style.font;
}

void LBaseView::setPos(LInt x, LInt y)
{
    m_x = x;
    m_y = y;
}

void LBaseView::setSize(LInt width, LInt height)
{
    m_width = width;
    m_height = height;
}

void LBaseView::translate(LInt x, LInt y)
{
    m_x += x;
    m_y += y;
}

LInt LBaseView::getXpos() const
{
    return m_x;
}

LInt LBaseView::getYpos() const
{
    return m_y;
}

LInt LBaseView::getWidth() const
{
    return m_width;
}

LInt LBaseView::getHeight() const
{
    return m_height;
}

void LBaseView::setWidth(LInt width)
{
    m_width = width;
}

void LBaseView::setHeight(LInt height)
{
    m_height = height;
}

void LBaseView::setPadding(LInt leftPadding, LInt topPadding)
{
    m_leftPadding = leftPadding;
    m_topPadding = topPadding;
}

void LBaseView::setVisible(LBool visible)
{
    m_visible = visible;
}

int LBaseView::getEndX()
{
    return m_x + m_width;
}

int LBaseView::getBottomY()
{
    return m_y + m_height;
}

void LBaseView::setXpos(LInt x)
{
    m_x = x;
}

void LBaseView::setYpos(LInt y)
{
    m_y = y;
}

void LBaseView::setTagName(const String& tagName)
{
    m_tagName = tagName;
}

const String& LBaseView::getTagName() const
{
    return m_tagName;
}
}

/*
 * FormView.cpp
 *
 *  Created on: 2011-7-8
 *      Author: yanbo
 */

#include "FormView.h"

namespace yanbo {

FormView::FormView(
    const String& id,
    const String& name,
    const String& inValue,
    const String& inTitle)
    : InlineView(id, LTrue)
    , m_name(name)
{
    m_value = inValue;
    m_title = inTitle;
    m_style.transparent = LFalse;
}

FormView::~FormView()
{
}

void FormView::setValue(const String& inValue)
{
    m_value = inValue;
}

const String& FormView::getValue() const
{
    return m_value;
}

void FormView::setTitle(const String& title)
{
    m_title = title;
}

const String& FormView::getTitle() const
{
    return m_title;
}

void FormView::setName(const String& name)
{
    m_name = name;
}

const String& FormView::getName() const
{
    return m_name;
}
}

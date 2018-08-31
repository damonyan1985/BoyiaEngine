/*
 * FormView.cpp
 *
 *  Created on: 2011-7-8
 *      Author: yanbo
 */

#include "FormView.h"

//#define MAX_PASSWORD 32
//#define MAX_TEXTAREA 1024

namespace yanbo
{

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
	m_mimeType = _CS("application/x-www-form-urlencoded");
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

void FormView::setParentForm(HtmlForm* form)
{
	m_parentForm = form;
}

const String& FormView::getMimeType() const
{
	return m_mimeType;
}

const String FormView::getEncodingParams() const
{
    return _CS("");	
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

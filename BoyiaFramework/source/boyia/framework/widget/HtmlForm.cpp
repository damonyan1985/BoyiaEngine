/*
 * HtmlForm.cpp
 *
 *  Created on: 2011-7-26
 *      Author: yanbo
 */

#include "HtmlForm.h"
#include "InputView.h"

namespace yanbo {

HtmlForm::HtmlForm(
    const String& id,
    const String& action,
    const String& method,
    const String& encoding,
    LBool selectable)
    : BlockView(id, selectable)
    , m_action(action)
{
    if (method.GetLength() == 0) {
        m_method = _CS("GET");
    } else {
        m_method = method;
        m_method.ToUpper();
    }

    if (encoding.GetLength() == 0) {
        m_encoding = _CS("application/x-www-form-urlencoded");
    } else {
        m_encoding = encoding;
        if (m_encoding.CompareNoCase(_CS("multipart/form-data"))) {
            m_boundaryString = _CS("---------------------------7d816b1e15035a");
        }
    }
}

HtmlForm::~HtmlForm()
{
}

void HtmlForm::addFormView(FormView* aFormView)
{
    m_formList.push(aFormView);
}

const FormViewList& HtmlForm::getFormViews() const
{
    return m_formList;
}

void HtmlForm::layout(RenderContext& rc)
{
    BlockView::layout();
}

void HtmlForm::reset()
{
}

String HtmlForm::getAction()
{
    return m_action;
}

String HtmlForm::getMethod()
{
    return m_method;
}

LBool HtmlForm::hasPassword()
{
    FormViewList::Iterator iter = m_formList.begin();
    FormViewList::Iterator iterEnd = m_formList.end();
    for (; iter != iterEnd; ++iter) {
        FormView* fItem = *iter;
        if (fItem->getTagType() == HtmlTags::INPUT) {
            InputView* inpItem = (InputView*)fItem;
            if (inpItem->getInputType() == InputView::kInputPassword) {
                return LTrue;
            }
        }
    }

    return LFalse;
}

String HtmlForm::getEncoding()
{
    if (m_boundaryString.GetLength() == 0) {
        return m_encoding;
    }

    return m_encoding + _CS("; boundary=") + m_boundaryString;
}

String HtmlForm::getItemValue(const String& itemName)
{
    FormView* fItem = getFormView(itemName);
    if (fItem) {
        return fItem->getValue();
    }

    return _CS("");
}

String HtmlForm::getItemMimeType(const String& itemName)
{
    FormView* fItem = getFormView(itemName);
    if (fItem) {
        return fItem->getMimeType();
    }

    return _CS("");
}

void HtmlForm::setItemValue(const String& name, const String& value)
{
    FormView* fItem = getFormView(name);
    if (fItem) {
        return fItem->setValue(value);
    }
}

FormView* HtmlForm::getFormView(const String& itemName)
{
    FormViewList::Iterator iter = m_formList.begin();
    FormViewList::Iterator iterEnd = m_formList.end();
    for (; iter != iterEnd; ++iter) {
        FormView* fItem = *iter;
        if (fItem->getName().CompareNoCase(itemName)) {
            return fItem;
        }
    }

    return NULL;
}

void HtmlForm::unCheckRadioButtons(const String& name)
{
    FormViewList::Iterator iter = m_formList.begin();
    FormViewList::Iterator iterEnd = m_formList.end();
    for (; iter != iterEnd; ++iter) {
        FormView* fItem = *iter;
        if (fItem->getTagType() == HtmlTags::INPUT) {
            InputView* inpItem = (InputView*)fItem;
            inpItem->setSelected(LFalse);
        }
    }
}

LInt HtmlForm::getLastSubmitTime()
{
    return m_lastSubmitTime;
}

void HtmlForm::postForm(String& outstream)
{
    LBool isMultiPart = m_encoding.CompareNoCase(_CS("multipart/form-data"));
    LBool firstParam = LTrue;
}

String HtmlForm::encodeFormViews()
{
    return _CS("");
}
}

#include "VDocument.h"

namespace yanbo {
VNode::~VNode()
{
}

LBool VNode::isElement()
{
    return LTrue;
}

LBool VNode::isText()
{
    return LFalse;
}

VElement::VElement()
{
}

LVoid VElement::setId(const String& id)
{
    m_id = id;
}
LVoid VElement::setClass(const String& className)
{
    m_class = className;
}
LVoid VElement::setTag(LInt tag)
{
    m_tag = tag;
}

LVoid VElement::add(VNode* node)
{
    m_nodes.addElement(node);
}

LBool VText::isElement()
{
    return LFalse;
}

LBool VText::isText()
{
    return LTrue;
}

LVoid VText::setText(const String& text)
{
    m_value = text;
}

VDocument::VDocument()
    : m_root(NULL)
{
}

LVoid VDocument::setRoot(VNode* node)
{
    m_root = node;
}
}
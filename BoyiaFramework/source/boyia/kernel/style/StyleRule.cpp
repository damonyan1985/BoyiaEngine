/*
 * StyleRule.cpp
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#include "StyleRule.h"
#include "LColorUtil.h"
#include "LGdi.h"
#include "SalLog.h"
#include "StyleTags.h"

namespace util {

SimpleSelector::SimpleSelector()
    : m_relation(Descendant)
    , m_pseudoType(PseudoNotParsed)
{
}

SimpleSelector::~SimpleSelector()
{
}

void SimpleSelector::setSelectorText(const String& selectorText)
{
    m_selectorText = selectorText;

    int index = m_selectorText.FindNoCase(_CS(":"));
    if (index > 0) {
        setMatch(SimpleSelector::PseudoClass);
        // such as a:active
        String pseudoName = m_selectorText.Left(index);
        String pseudoClass = m_selectorText.Right(m_selectorText.GetLength() - index - 1);

        if (pseudoClass == _CS("active")) {
            setPseudoType(SimpleSelector::PseudoActive);
        } else if (pseudoClass == _CS("visited")) {
            setPseudoType(SimpleSelector::PseudoVisited);
        } else if (pseudoClass == _CS("link")) {
            setPseudoType(SimpleSelector::PseudoLink);
        } else if (pseudoClass == _CS("hover")) {
            setPseudoType(SimpleSelector::PseudoHover);
        } else {
            setPseudoType(SimpleSelector::PseudoNotParsed);
        }

        return;
    }

    if (m_selectorText.StartWith(_CS("#"))) {
        setMatch(SimpleSelector::Id);
    } else if (m_selectorText.StartWith(_CS("."))) {
        setMatch(SimpleSelector::Class);
    } else {
        setMatch(SimpleSelector::Tag);
    }
}

void SimpleSelector::setMatch(LInt8 match)
{
    m_match = match;
}

LInt8 SimpleSelector::getMatch() const
{
    return m_match;
}

const String& SimpleSelector::getSelectorText() const
{
    return m_selectorText;
}

void SimpleSelector::setRelation(LInt8 relation)
{
    m_relation = relation;
}

LInt8 SimpleSelector::getRelation() const
{
    return m_relation;
}

void SimpleSelector::setPseudoType(LInt8 pseudoType)
{
    m_pseudoType = pseudoType;
}

LInt8 SimpleSelector::getPseudoType() const
{
    return m_pseudoType;
}

CssPropertyValue::CssPropertyValue()
    : intVal(0)
{
    resetSpecificity();
}

CssPropertyValue::CssPropertyValue(const CssPropertyValue& proValue)
{
    intVal = proValue.intVal;
    strVal = proValue.strVal;

    m_specificity.m_id = proValue.m_specificity.m_id;
    m_specificity.m_important = proValue.m_specificity.m_important;
    m_specificity.m_classOrPseudo = proValue.m_specificity.m_classOrPseudo;
    m_specificity.m_tag = proValue.m_specificity.m_tag;
}

CssPropertyValue::~CssPropertyValue()
{
}

const CssPropertyValue& CssPropertyValue::operator=(const CssPropertyValue& proValue)
{
    intVal = proValue.intVal;
    strVal = proValue.strVal;

    m_specificity.m_id = proValue.m_specificity.m_id;
    m_specificity.m_important = proValue.m_specificity.m_important;
    m_specificity.m_classOrPseudo = proValue.m_specificity.m_classOrPseudo;
    m_specificity.m_tag = proValue.m_specificity.m_tag;
    return *this;
}

void CssPropertyValue::resetSpecificity()
{
    m_specificity.m_important = 0;
    m_specificity.m_id = 0;
    m_specificity.m_tag = 0;
    m_specificity.m_classOrPseudo = 0;
}

void CssPropertyValue::computeSpecificity(const Selector* selector)
{
    CssPropertyValue::computeSpecificity(selector, m_specificity);
}

void CssPropertyValue::computeSpecificity(const Selector* selector, PropertySpecificity& specificity)
{
    LInt num = selector->size();
    for (LInt i = 0; i < num; i++) {
        switch (selector->elementAt(i)->getMatch()) {
        case SimpleSelector::Id: {
            specificity.m_id += 1;
        } break;
        case SimpleSelector::Class:
        case SimpleSelector::PseudoClass: {
            specificity.m_classOrPseudo += 1;
        } break;
        case SimpleSelector::Tag: {
            specificity.m_tag += 1;
        } break;
        default:
            break;
        }
    }
}

LBool CssPropertyValue::compareSpecificity(const PropertySpecificity& specificity1, const PropertySpecificity& specificity2)
{
    KLOG("CssPropertyValue::compareSpecificity");
    if (specificity1.m_important > 0)
        return LTrue;

    if (specificity2.m_important > 0)
        return LFalse;

    KLOG("CssPropertyValue::compareSpecificity1");
    if (specificity1.m_id > specificity2.m_id) {
        KLOG("CssPropertyValue::compareSpecificity");
        return LTrue;
    }

    if (specificity1.m_id == specificity2.m_id) {
        KLOG("CssPropertyValue::compareSpecificity");
        if (specificity1.m_classOrPseudo > specificity2.m_classOrPseudo) {
            return LTrue;
        }

        if (specificity1.m_classOrPseudo == specificity2.m_classOrPseudo
            && specificity1.m_tag > specificity2.m_tag) {
            KLOG("CssPropertyValue::compareSpecificity");
            return LTrue;
        }
    }

    return LFalse;
}

void CssPropertyValue::setSpecificity(const CssPropertyValue::PropertySpecificity& specificity)
{
    m_specificity.m_id = specificity.m_id;
    m_specificity.m_important = specificity.m_important;
    m_specificity.m_tag = specificity.m_tag;
    m_specificity.m_classOrPseudo = m_specificity.m_classOrPseudo;
}

const CssPropertyValue::PropertySpecificity& CssPropertyValue::getSpecificity() const
{
    return m_specificity;
}

StyleRule::StyleRule()
    : m_selectorGroup(kBoyiaNull)
{
}

StyleRule::~StyleRule()
{
    KLOG("StyleRule::~StyleRule()");
    if (m_selectorGroup) {
        LInt groupIdx = m_selectorGroup->size();
        while (groupIdx) {
            Selector* sel = m_selectorGroup->elementAt(--groupIdx);
            LInt selIdx = sel->size();
            while (selIdx) {
                SimpleSelector* simpleSelector = sel->elementAt(--selIdx);
                delete simpleSelector;
            }

            delete sel;
        }

        delete m_selectorGroup;
    }
}

void StyleRule::construct()
{
}

StyleRule* StyleRule::New()
{
    StyleRule* rule = new StyleRule();
    if (rule) {
        rule->construct();
    }
    return rule;
}

StyleRule* StyleRule::New(StyleRule& rule)
{
    StyleRule* newRule = new StyleRule();
    if (newRule) {
        newRule->construct(rule);
    }

    return newRule;
}

void StyleRule::construct(StyleRule& rule)
{
    m_properties = rule.getProperties();
    m_selectorGroup = rule.getSelectorGroup();
}

void StyleRule::addProperty(LInt property, LInt value)
{
    CssPropertyValue v;
    v.intVal = value;
    m_properties.put(property, v);
}

void StyleRule::addProperty(LInt property, const String& value)
{
    CssPropertyValue v;
    v.intVal = 0;
    v.strVal = value;
    m_properties.put(property, v);
}

CssPropertyValue& StyleRule::getPropertyValue(LInt property)
{
    return m_properties[property];
}

const AttributeMap& StyleRule::getProperties() const
{
    return m_properties;
}

void StyleRule::copyPropertiesFrom(const StyleRule* rule)
{
    if (kBoyiaNull != rule) {
        AttributeMap::Iterator iter = rule->getProperties().begin();
        AttributeMap::Iterator iterEnd = rule->getProperties().end();
        for (; iter != iterEnd; ++iter) {
            BOYIA_LOG("StyleRule::copyPropertiesFrom key=%d prop size=%d", (*iter)->getKey(), m_properties.size());
            m_properties[(*iter)->getKey()].getSpecificity();
            KLOG("StyleRule::copyPropertiesFrom 1");
            (*iter)->getValue().getSpecificity();
            KLOG("StyleRule::copyPropertiesFrom 2");
            if (!CssPropertyValue::compareSpecificity(m_properties[(*iter)->getKey()].getSpecificity(), (*iter)->getValue().getSpecificity())) {
                KLOG("StyleRule::copyPropertiesFrom 3");
                m_properties.put((*iter)->getKey(), (*iter)->getValue());
                KLOG("StyleRule::copyPropertiesFrom 4");
            }

            KLOG("StyleRule::copyPropertiesFrom end");
        }
    }
}

// last style will be send to htmlitem object
void StyleRule::createStyle(Style& style)
{
    style.transparent = LTrue;

    AttributeMap::Iterator iter = m_properties.begin();
    AttributeMap::Iterator iterEnd = m_properties.end();
    for (; iter != iterEnd; ++iter) {
        setStyleProperties(style, (*iter)->getKey(), (*iter)->getValue());
    }
}

SelectorGroup* StyleRule::getSelectorGroup() const
{
    return m_selectorGroup;
}

const AttributeMap* StyleRule::getPropertiesPtr() const
{
    return &m_properties;
}

void StyleRule::setStyleProperties(Style& style, LInt property,
    const CssPropertyValue& value)
{
    switch (property) {
    case StyleTags::ALIGN: {
        style.align = value.intVal;
    } break;
    case StyleTags::BACKGROUND_COLOR: {
        style.bgColor = util::LColorUtil::parseArgbInt(value.intVal);
        style.transparent = LFalse;
        BOYIA_LOG("style.m_bgColor=%x", value.intVal);
    } break;
    case StyleTags::BACKGROUND_IMAGE: {
        style.bgImageUrl = value.strVal;
        style.transparent = LFalse;
    } break;
    case StyleTags::COLOR: {
        BOYIA_LOG("style Color=%x", value.intVal);
        style.color = LColorUtil::parseArgbInt(value.intVal);
    } break;
    case StyleTags::FONT_STYLE: {
        style.font.setFontStyle((LFont::FontStyle)value.intVal);
    } break;
    case StyleTags::FONT_WEIGHT: {
        style.font.setFontStyle((LFont::FontStyle)value.intVal);
    } break;
    case StyleTags::FONT_FAMILY: {
        style.font.setFamily(value.strVal);
    } break;
    case StyleTags::LEFT: {
        BOYIA_LOG("ImageItem::layout style.m_left=%d", value.intVal);
        style.left = value.intVal;
    } break;
    case StyleTags::TOP: {
        style.top = value.intVal;
    } break;
    case StyleTags::POSITION: {
        style.positionType = value.intVal;
    } break;
    case StyleTags::FONT_SIZE: {
        style.font.setFontSize(value.intVal);
    } break;
    case StyleTags::BORDER_STYLE: {
        if (value.intVal == LGraphicsContext::kSolidPen) {
            style.border().topWidth = style.border().topWidth > 1 ? style.border().topWidth : 1;
            style.border().leftWidth = style.border().leftWidth > 1 ? style.border().leftWidth : 1;
            style.border().bottomWidth = style.border().bottomWidth > 1 ? style.border().bottomWidth : 1;
            style.border().rightWidth = style.border().rightWidth > 1 ? style.border().rightWidth : 1;
        }
    } break;
    case StyleTags::TEXT_ALIGN: {
        style.textAlignement = value.intVal;
    } break;
    case StyleTags::MARGIN_LEFT: {
        style.margin().leftMargin = value.intVal;
    } break;
    case StyleTags::MARGIN_TOP: {
        style.margin().topMargin = value.intVal;
    } break;
    case StyleTags::MARGIN_RIGHT: {
        style.margin().rightMargin = value.intVal;
    } break;
    case StyleTags::MARGIB_BOTTOM: {
        style.margin().bottomMargin = value.intVal;
    } break;
    case StyleTags::PADDING_LEFT: {
        style.padding().leftPadding = value.intVal;
    } break;
    case StyleTags::PADDING_TOP: {
        style.padding().topPadding = value.intVal;
    } break;
    case StyleTags::PADDING_RIGHT: {
        style.padding().rightPadding = value.intVal;
    } break;
    case StyleTags::PADDING_BOTTOM: {
        style.padding().bottomPadding = value.intVal;
    } break;
    case StyleTags::BORDER_BOTTOM_WIDTH: {
        style.border().bottomWidth = value.intVal;
    } break;
    case StyleTags::BORDER_TOP_WIDTH: {
        style.border().topWidth = value.intVal;
    } break;
    case StyleTags::BORDER_TOP_STYLE: {
        style.border().topStyle = value.intVal;
        if (value.intVal == LGraphicsContext::kSolidPen) {
            style.border().topWidth = 1;
        }
    } break;
    case StyleTags::BORDER_BOTTOM_STYLE: {
        style.border().bottomStyle = value.intVal;
        if (value.intVal == LGraphicsContext::kSolidPen) {
            style.border().bottomWidth = 1;
        }
    } break;
    case StyleTags::BORDER_LEFT_STYLE: {
        style.border().leftStyle = value.intVal;
        if (value.intVal == LGraphicsContext::kSolidPen) {
            style.border().leftWidth = 1;
        }
    } break;
    case StyleTags::BORDER_RIGHT_STYLE: {
        style.border().rightStyle = value.intVal;
        if (value.intVal == LGraphicsContext::kSolidPen) {
            style.border().rightWidth = 1;
        }
    } break;
    case StyleTags::BORDER_COLOR: {
        style.border().leftColor = value.intVal;
        style.border().topColor = value.intVal;
        style.border().bottomColor = value.intVal;
        style.border().rightColor = value.intVal;
    } break;
    case StyleTags::BORDER_LEFT_COLOR: {
        style.border().leftColor = value.intVal;
    } break;
    case StyleTags::BORDER_TOP_COLOR: {
        style.border().topColor = value.intVal;
    } break;
    case StyleTags::BORDER_BOTTOM_COLOR: {
        style.border().bottomColor = value.intVal;
    } break;
    case StyleTags::BORDER_RIGHT_COLOR: {
        style.border().rightColor = value.intVal;
    } break;
    case StyleTags::DISPLAY: {
        style.displayType = value.intVal;
    } break;
    case StyleTags::WIDTH: {
        style.width = value.intVal;
    } break;
    case StyleTags::HEIGHT: {
        style.height = value.intVal;
    } break;
    case StyleTags::SCALE: {
        style.scale = ((float)value.intVal) / 100;
    } break;
    case StyleTags::Z_INDEX: {
        style.zindex = value.intVal;
    } break;
    case StyleTags::FOCUSABLE: {
        BOYIA_LOG("StyleTags::FOCUSABLE result=%d", value.intVal);
        style.focusable = value.intVal;
    } break;
    case StyleTags::FLEX_DIRECTION: {
        style.flexDirection = value.intVal;
    } break;
    case StyleTags::BORDER_RADIUS: {
        style.radius().topLeftRadius = value.intVal;
        style.radius().topRightRadius = value.intVal;
        style.radius().bottomLeftRadius = value.intVal;
        style.radius().bottomRightRadius = value.intVal;
    } break;
    case StyleTags::BORDER_TOP_LEFT_RADIUS: {
        style.radius().topLeftRadius = value.intVal;
    } break;
    case StyleTags::BORDER_TOP_RIGHT_RADIUS: {
        style.radius().topRightRadius = value.intVal;
    } break;
    case StyleTags::BORDER_BOTTOM_LEFT_RADIUS: {
        style.radius().bottomLeftRadius = value.intVal;
    } break;
    case StyleTags::BORDER_BOTTOM_RIGHT_RADIUS: {
        style.radius().bottomRightRadius = value.intVal;
    } break;
    default:
        break;
    }
}

LBool StyleRule::isPropertyEmpty()
{
    return m_properties.isEmpty();
}

void StyleRule::setSelectorGroup(SelectorGroup* group)
{
    m_selectorGroup = group;
}
}

/*
 * StyleManager.cpp
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */

#include "StyleManager.h"
#include "SalLog.h"
#include "StyleTags.h"

namespace util {

StyleManager::StyleManager()
    : m_doctree(new Doctree(20))
{
}

StyleManager::~StyleManager()
{
    RuleList::Iterator iter = m_ruleList.begin();
    RuleList::Iterator endIter = m_ruleList.end();

    for (; iter != endIter; ++iter) {
        delete *iter;
    }

    m_ruleList.clear();

    LInt idx = m_doctree->size();
    while (idx) {
        delete m_doctree->elementAt(--idx);
    }

    delete m_doctree;
}

void StyleManager::addStyleRule(StyleRule* rule)
{
    m_ruleList.push(rule);
}

// get Specified htmitem's StyleRule
StyleRule* StyleManager::getStyleRule()
{
    StyleRule* cssStyle = StyleRule::New();

    RuleList::Iterator iter = m_ruleList.begin();
    RuleList::Iterator endIter = m_ruleList.end();

    LBool isNull = LTrue;
    KLOG("StyleManager::getStyleRule1");
    for (; iter != endIter; ++iter) {
        StyleRule* newRule = matchRule(*iter);
        if (newRule != kBoyiaNull) {
            isNull = LFalse;
            cssStyle->copyPropertiesFrom(newRule);
        }
    }

    KLOG("StyleManager::getStyleRule2");
    if (isNull) {
        delete cssStyle;
        cssStyle = kBoyiaNull;
    }

    return cssStyle;
}

LBool StyleManager::matchPrepare(Selector* sel)
{
    if (!sel->size()) {
        return LFalse;
    }

    //KFORMATLOG("StyleManager::matchPrepare sel->size()=%d simpler=%d", index, (LIntPtr)simpler);
    const String& elem = sel->elementAt(sel->size() - 1)->getSelectorText();
    DoctreeNode* docElem = m_doctree->elementAt(m_doctree->size() - 1);
    if ((elem.CompareNoCase(docElem->tagId) && !elem.CompareNoCase(_CS("#")))
        || elem.CompareNoCase(docElem->tagName)
        || (docElem->tagClassArray.contains(elem) && !elem.CompareNoCase(_CS(".")))) {
        return LTrue;
    }

    return LFalse;
}

StyleRule* StyleManager::matchRule(StyleRule* rule)
{
    StyleRule* newCssStyle = kBoyiaNull;
    if (rule != kBoyiaNull) {
        SelectorGroup* selectorGroup = rule->getSelectorGroup();
        Selector* targetSelector = kBoyiaNull;
        CssPropertyValue::PropertySpecificity specificity;
        LInt selId = selectorGroup->size();

        while (selId) {
            Selector* sel = selectorGroup->elementAt(--selId);
            if (!matchPrepare(sel)) {
                continue;
            }

            LBool find = LFalse;
            LInt simIdx = sel->size();
            LInt docIndex = m_doctree->size() - 1;
            while (simIdx) {
                find = LFalse;
                if (docIndex < 0) {
                    break;
                }

                const String& elem = sel->elementAt(--simIdx)->getSelectorText();
                while (docIndex >= 0) {
                    DoctreeNode* docElem = m_doctree->elementAt(docIndex--);
                    if ((elem.StartWith(_CS("#")) && elem.CompareNoCase(docElem->tagId))
                        || (elem.StartWith(_CS(".")) && docElem->tagClassArray.contains(elem))
                        || elem.CompareNoCase(docElem->tagName)) {
                        // find one
                        find = LTrue;
                        break;
                    }
                }

                if (!find) {
                    // 如果找到了继续对selector的下一个元素进行查找,
                    // 如果找不到则直接调出循环,find代表查找失败
                    break;
                }
            }

            if (find) {
                // have found!!!
                targetSelector = sel;

                CssPropertyValue::PropertySpecificity tmpSpecificity;
                CssPropertyValue::computeSpecificity(sel, tmpSpecificity);
                LBool result = CssPropertyValue::compareSpecificity(specificity, tmpSpecificity);
                if (!result) {
                    specificity = tmpSpecificity;
                }

                // 继续查找优先级更高的specificity
            }
        }

        if (specificity.m_id != 0 || specificity.m_tag != 0 || specificity.m_classOrPseudo != 0) {
            newCssStyle = rule;

            AttributeMap::Iterator iter = newCssStyle->getProperties().begin();
            AttributeMap::Iterator iterEnd = newCssStyle->getProperties().end();
            for (; iter != iterEnd; ++iter) {
                (*iter)->getValue().setSpecificity(specificity);
            }
        }
    }

    return newCssStyle;
}

void StyleManager::pushDoctreeNode(DoctreeNode* node)
{
    m_doctree->push(node);
}

void StyleManager::pushDoctreeNode(const String& tagId, const ClassArray& tagClass, const String& tagName)
{
    m_doctree->push(new DoctreeNode(tagId, tagClass, tagName));
}

void StyleManager::popDoctreeNode()
{
    delete m_doctree->pop();
}

// inherited parent style
StyleRule* StyleManager::createNewStyleRule(const StyleRule* parentRule, StyleRule* childRule)
{
    KLOG("StyleManager::createNewStyleRule");
    StyleRule* newStyleRule = StyleRule::New();
    if (parentRule) {
        const AttributeMap* properties = parentRule->getPropertiesPtr();
        AttributeMap::Iterator iter = properties->begin();
        AttributeMap::Iterator iterEnd = properties->end();
        for (; iter != iterEnd; ++iter) {
            if ((*iter)->getKey() > StyleTags::STYLE_NULL) //  iter.getKey() > 0 can be inherited
            {
                LInt key = (*iter)->getKey();
                LInt styleNull = StyleTags::STYLE_NULL;
                KFORMATLOG("StyleManager::createNewStyleRule inherited key=%d styleNull=%d", key, styleNull);

                if ((*iter)->getValue().strVal.GetLength() > 0) {
                    newStyleRule->addProperty((*iter)->getKey(), (*iter)->getValue().strVal);
                } else {
                    newStyleRule->addProperty((*iter)->getKey(), (*iter)->getValue().intVal);
                }
            }
        }
    }

    if (childRule) {
        const AttributeMap* properties = childRule->getPropertiesPtr();
        AttributeMap::Iterator iter = properties->begin();
        AttributeMap::Iterator iterEnd = properties->end();
        for (; iter != iterEnd; ++iter) {
            KDESLOG((*iter)->getKey());
            if ((*iter)->getValue().strVal.GetLength() > 0) {
                newStyleRule->addProperty((*iter)->getKey(), (*iter)->getValue().strVal);
            } else {
                newStyleRule->addProperty((*iter)->getKey(), (*iter)->getValue().intVal);
            }
        }

        delete childRule;
    }

    return newStyleRule;
}
}

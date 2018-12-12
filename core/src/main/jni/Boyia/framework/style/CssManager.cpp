/*
 * CssManager.cpp
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */

#include "CssManager.h"
#include "CssTags.h"
#include "SalLog.h"

namespace util
{

CssManager::CssManager()
    : m_doctree(new Doctree(20))
{
}

CssManager::~CssManager()
{
	RuleList::Iterator iter = m_ruleList.begin();
	RuleList::Iterator endIter = m_ruleList.end();
	
	for (; iter != endIter; ++iter)
	{
	    delete *iter;
	}
	
	m_ruleList.clear();
	
	LInt idx = m_doctree->size();
	while (idx)
	{
	    delete m_doctree->elementAt(--idx);
	}

	delete m_doctree;
}

void CssManager::addCssRule(CssRule* rule)
{
	m_ruleList.push(rule);
}

// get Specified htmitem's cssrule
CssRule* CssManager::getCssRule()
{
	CssRule* cssStyle = CssRule::New();
	
	RuleList::Iterator iter = m_ruleList.begin();
	RuleList::Iterator endIter = m_ruleList.end();
	
	LBool isNull = LTrue;
	KLOG("CssManager::getCssRule1");
	for (; iter != endIter; ++iter)
	{
	    CssRule* newRule = matchRule(*iter);
	    if (newRule != NULL)
	    {
	        isNull = LFalse;
	        cssStyle->copyPropertiesFrom(newRule);
	    }
	}

	KLOG("CssManager::getCssRule2");
	if (isNull)
	{
	    delete cssStyle;
	    cssStyle = NULL;
	}
	
	return cssStyle;
}

LBool CssManager::matchPrepare(Selector* sel)
{
	if (!sel->size())
	{
		return LFalse;
	}

	//KFORMATLOG("CssManager::matchPrepare sel->size()=%d simpler=%d", index, (LIntPtr)simpler);
	const String& elem = sel->elementAt(sel->size() - 1)->getSelectorText();
	DoctreeNode* docElem = m_doctree->elementAt(m_doctree->size() - 1);
	if ((elem.CompareNoCase(docElem->m_tagId) && !elem.CompareNoCase(_CS("#")))
		|| elem.CompareNoCase(docElem->m_tagName)
		|| (docElem->m_classArray.contains(elem) && !elem.CompareNoCase(_CS(".")))
			)
	{
		return LTrue;
	}

	return LFalse;
}

CssRule* CssManager::matchRule(CssRule* rule)
{
	CssRule* newCssStyle = NULL;
	if (rule != NULL)
	{
	    SelectorGroup* selectorGroup = rule->getSelectorGroup();
		Selector* targetSelector = NULL;
		CssPropertyValue::PropertySpecificity specificity;
		LInt selId = selectorGroup->size();

		while (selId)
		{
			Selector* sel = selectorGroup->elementAt(--selId);
			if (!matchPrepare(sel))
			{
				continue;
			}

			LBool find = LFalse;
			LInt simIdx = sel->size();
			LInt docIndex = m_doctree->size() - 1;
			while (simIdx)
			{
				find = LFalse;
				if (docIndex < 0)
				{
					break;
				}

				const String& elem = sel->elementAt(--simIdx)->getSelectorText();
				while (docIndex >= 0)
				{            		
        			DoctreeNode* docElem = m_doctree->elementAt(docIndex--);
        			if ((elem.StartWith(_CS("#")) && elem.CompareNoCase(docElem->m_tagId))
        					|| (elem.StartWith(_CS(".")) && docElem->m_classArray.contains(elem))
        					|| elem.CompareNoCase(docElem->m_tagName)
        					)
        			{
        				// find one
        				find = LTrue;
        				break;
        			}
        		}

        		if (!find)
				{
        			// 如果找到了继续对selector的下一个元素进行查找,
					// 如果找不到则直接调出循环,find代表查找失败
					break;
        		}
			}
	    
			if (find)
			{
				// have found!!!
				targetSelector = sel;
				
				CssPropertyValue::PropertySpecificity tmpSpecificity;
				CssPropertyValue::computeSpecificity(sel, tmpSpecificity);
				LBool result = CssPropertyValue::compareSpecificity(specificity, tmpSpecificity);
				if (!result)
				{
					specificity = tmpSpecificity;
				}

				// 继续查找优先级更高的specificity
			}

		}

		if (specificity.m_id != 0 || specificity.m_tag != 0 || specificity.m_classOrPseudo != 0)
		{
			newCssStyle = rule;

			AttributeMap::Iterator iter = newCssStyle->getProperties().begin();
	        AttributeMap::Iterator iterEnd = newCssStyle->getProperties().end();
	        for (; iter != iterEnd; ++iter)
	        {
				(*iter)->getValue().setSpecificity(specificity);
	        }
		}
       
	}
	
	return newCssStyle;
}

void CssManager::pushDoctreeNode(DoctreeNode* node)
{
	m_doctree->push(node);
}

void CssManager::pushDoctreeNode(const String& tagId, const ClassArray& tagClass, const String& tagName)
{
	m_doctree->push(new DoctreeNode(tagId, tagClass, tagName));
}

void CssManager::popDoctreeNode()
{
	delete m_doctree->pop();
}

// inherited parent style
CssRule* CssManager::createNewCssRule(const CssRule* parentRule, CssRule* childRule)
{
	KLOG("CssManager::createNewCssRule");
	CssRule* newCssRule =  CssRule::New();
    if (NULL != parentRule)
    {    	
		const AttributeMap* properties = parentRule->getPropertiesPtr();
		AttributeMap::Iterator iter = properties->begin();
		AttributeMap::Iterator iterEnd = properties->end();
		for (; iter != iterEnd; ++iter)
		{
			if ((*iter)->getKey() > CssTags::STYLE_NULL) //  iter.getKey() > 0 can be inherited
			{
				LInt key = (*iter)->getKey();
				LInt styleNull = CssTags::STYLE_NULL;
				KFORMATLOG("CssManager::createNewCssRule inherited key=%d styleNull=%d", key, styleNull);

				if ((*iter)->getValue().strVal.GetLength() > 0)
				{
				    newCssRule->addProperty((*iter)->getKey(), (*iter)->getValue().strVal);
				}
				else
				{
				    newCssRule->addProperty((*iter)->getKey(), (*iter)->getValue().intVal);
				}
			}
		}
    }
    
    if (NULL != childRule)
    {    	
		const AttributeMap* properties = childRule->getPropertiesPtr();
		AttributeMap::Iterator iter = properties->begin();
		AttributeMap::Iterator iterEnd = properties->end();
		for (; iter != iterEnd; ++iter)
		{
			KDESLOG((*iter)->getKey());
		    if ((*iter)->getValue().strVal.GetLength() > 0)
			{
				newCssRule->addProperty((*iter)->getKey(), (*iter)->getValue().strVal);
			}
			else
			{
				newCssRule->addProperty((*iter)->getKey(), (*iter)->getValue().intVal);
			}
		}

		delete childRule;
    }

	return newCssRule;
}

}

/*
 * CssManager.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef CssManager_h
#define CssManager_h

#include "KList.h"
#include "CssRule.h"
#include "Stack.h"
#include "DoctreeNode.h"

namespace util
{

typedef KList<CssRule*>             RuleList;
typedef Stack<DoctreeNode*>         Doctree;
class CssManager
{
public:
	CssManager();
	~CssManager();
	
public:
	void addCssRule(CssRule* rule);
	CssRule* getCssRule();
	CssRule* matchRule(CssRule* rule);
	LBool matchPrepare(Selector* selector);
	CssRule* createNewCssRule(const CssRule* parentRule, CssRule* childRule);
	
	void pushDoctreeNode(DoctreeNode* node);
	void pushDoctreeNode(const String& tagId, const ClassArray& tagClass, const String& tagName);
	void popDoctreeNode();
	
protected:
	RuleList m_ruleList;
	Doctree* m_doctree;
};

}
#endif /* CssManager_h */

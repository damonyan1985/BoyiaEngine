/*
 * StyleManager.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef StyleManager_h
#define StyleManager_h

#include "DoctreeNode.h"
#include "KList.h"
#include "Stack.h"
#include "StyleRule.h"
#include "OwnerPtr.h"

namespace util {

typedef KList<StyleRule*> RuleList;
typedef Stack<DoctreeNode*> Doctree;
class StyleManager {
public:
    StyleManager();
    ~StyleManager();

public:
    void addStyleRule(StyleRule* rule);
    StyleRule* getStyleRule();
    StyleRule* matchRule(StyleRule* rule);
    LBool matchPrepare(Selector* selector);
    StyleRule* createNewStyleRule(const StyleRule* parentRule, StyleRule* childRule);

    void pushDoctreeNode(DoctreeNode* node);
    void pushDoctreeNode(const String& tagId, const ClassArray& tagClass, const String& tagName);
    void popDoctreeNode();

protected:
    RuleList m_ruleList;
    OwnerPtr<Doctree> m_doctree;
};
}
#endif /* StyleManager_h */

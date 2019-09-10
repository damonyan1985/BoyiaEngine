/**
 * 
 * VDOM For BoyiaUI Framework
 * Author Yanbo
 * 
*/

#ifndef VDocument_h
#define VDocument_h

#include "KVector.h"
#include "OwnerPtr.h"
#include "UtilString.h"

namespace yanbo {
class VNode {
public:
    virtual ~VNode();
    virtual LBool isElement();
    virtual LBool isText();
};

class VElement : public VNode {
public:
    VElement();
    LVoid setId(const String& id);
    LVoid setClass(const String& className);
    LVoid setTag(LInt tag);
    LVoid add(VNode* node);

private:
    String m_id;
    String m_class;
    LInt m_tag;
    KVector<OwnerPtr<VNode>> m_nodes;
};

class VText : public VNode {
public:
    LVoid setText(const String& text);
    virtual LBool isElement();
    virtual LBool isText();

private:
    String m_value;
};

class VDocument {
public:
    VDocument();
    LVoid setRoot(VNode* node);

private:
    OwnerPtr<VNode> m_root;
};
}

#endif
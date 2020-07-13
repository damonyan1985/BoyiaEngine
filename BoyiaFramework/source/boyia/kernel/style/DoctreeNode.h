/*
 *  DoctreeNode.h
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */

#ifndef DoctreeNode_h
#define DoctreeNode_h

#include "KVector.h"
#include "UtilString.h"
namespace util {

typedef KVector<String> ClassArray;
class DoctreeNode {
public:
    DoctreeNode();
    DoctreeNode(const String& id,
        const ClassArray& classArray, const String& name);

    ~DoctreeNode();

public:
    String tagName; // such as input,html,body
    String tagId; // such as #id
    ClassArray tagClassArray; // such as .class
};
}
#endif /* DOCTREENODE_H_ */

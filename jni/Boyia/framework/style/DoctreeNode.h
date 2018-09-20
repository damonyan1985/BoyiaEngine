/*
 *  DoctreeNode.h
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */

#ifndef DoctreeNode_h
#define DoctreeNode_h

#include "UtilString.h"
#include "KVector.h"
namespace util
{

typedef KVector<String> ClassArray;
class DoctreeNode
{
public:
	DoctreeNode();
	DoctreeNode(const String& tagId,
			const ClassArray& tagClassArray, const String& tagName);
	
	~DoctreeNode();
	
public:
	String    m_tagName;     // such as input,html,body
	String    m_tagId;       // such as #id
	ClassArray m_classArray;  // such as .class
};

}
#endif /* DOCTREENODE_H_ */

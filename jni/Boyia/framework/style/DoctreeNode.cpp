/*
 * DoctreeNode.cpp
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */
#include "DoctreeNode.h"
namespace util
{

DoctreeNode::DoctreeNode()
{	
}

DoctreeNode::DoctreeNode(const String& tagId,
		const ClassArray& tagClassArray, 
		const String& tagName)
    : m_tagName(tagName, LFalse)
    , m_tagId(tagId, LFalse)
    , m_classArray(tagClassArray, LFalse)
{
}

DoctreeNode::~DoctreeNode()
{	
}

}

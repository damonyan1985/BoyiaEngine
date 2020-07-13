/*
 * DoctreeNode.cpp
 *
 *  Created on: 2011-6-29
 *      Author: yanbo
 */
#include "DoctreeNode.h"
namespace util {

DoctreeNode::DoctreeNode()
{
}

DoctreeNode::DoctreeNode(const String& id,
    const ClassArray& classArray,
    const String& name)
    : tagName(name, LFalse)
    , tagId(id, LFalse)
    , tagClassArray(classArray, LFalse)
{
}

DoctreeNode::~DoctreeNode()
{
}
}

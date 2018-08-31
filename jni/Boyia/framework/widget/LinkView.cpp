/*
 * LinkView.cpp
 *
 *  Created on: 2011-7-21
 *      Author: Administrator
 */

#include "LinkView.h"
#include "StringUtils.h"

namespace yanbo
{

LinkView::LinkView(
		const String& id,
		const String& url)
    : InlineView(id, LTrue)
{
	m_url = url;
}

LinkView::~LinkView()
{
	
}

LBool LinkView::isLink() const
{
	KLOG("LinkView::isLink()");
	return LTrue;
}

void LinkView::execute()
{
	if (m_url.GetLength() > 0 && NULL != m_itemListener)
	{
	    //m_itemListener->onClick(m_url, m_onClick);
	}
}

}

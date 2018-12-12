/*
 * LinkView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef LinkView_h
#define LinkView_h

#include "InlineView.h"

namespace yanbo
{

class LinkView : public InlineView
{
public:
    LinkView(
			const String& id,
			const String& url);
    
    ~LinkView();
    
public:
    virtual LBool isLink() const;
    
	virtual void execute();
	void setLinkFocus(HtmlView* item, const LBool selected);
	
private:
	String m_url;
};

}

#endif /* LinkView_H_ */

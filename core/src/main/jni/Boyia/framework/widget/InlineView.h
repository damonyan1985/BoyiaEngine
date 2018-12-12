/*
 * InlineView.h
 *
 *  Created on: 2012-8-06
 *      Author: yanbo
 */

#ifndef InlineView_h
#define InlineView_h

#include "HtmlView.h"

namespace yanbo
{

class InlineView : public HtmlView
{
public:
	InlineView(const String& id, LBool selectable);
	virtual ~InlineView();

	virtual void addChild(HtmlView* child);

public:
	virtual LBool isInline() const;

private:
	void addChildToContinuation(HtmlView* child);
	void addChildIgnoringContinuation(HtmlView* child);
};

}
#endif

/*
 * BlockView.h
 *
 *  Created on: 2011-6-30
 *   Modify on: 2012-12-02 All Copyright reserved
 *      Author: yanbo
 */

#ifndef BlockView_h
#define BlockView_h

#include "HtmlView.h"
namespace yanbo
{

// block item
class BlockView : public HtmlView
{
public:
	BlockView(const String& id, LBool selectable);
	
	virtual ~BlockView();
	
public:
	// layout method inline or block child
	void setChildrenInline(LBool isInline);
	LBool isChildrenInline();

	// add create anonymous or normal child
    virtual void addChild(HtmlView* child);
	void addChild(HtmlView* child, LBool isNotAnonymousBlock);

	// layer layout
	virtual void layout();
	void layoutBlock(LBool relayoutChildren);
	void layoutBlockChildren(LBool relayoutChildren);
	void layoutInlineChildren();
	// layout block child
	void layoutBlockChild(HtmlView* child, LayoutUnit& previousLogicalHeight);
	//void layoutInlineChild(HtmlView* child);
	void layoutPositionChild(HtmlView* child, LayoutUnit& previousLogicalHeight);
	void makeChildrenNonInline(HtmlView* block);
	BlockView* createAnonymousBlock();

	// paint child
	virtual void paint(LGraphicsContext& gc);
	virtual LBool isBlockView() const;

	void setIsAnonymousBlock(LBool isAnonymous);
	virtual LBool isAnonymousBlock();

	void setScrollPos(LInt x, LInt y);
	LInt getScrollXPos() const;
	LInt getScrollYPos() const;
	LBool canScroll() const;

	virtual LInt getHeight() const;
	// Can scroll max height
	LInt scrollHeight() const;

	LVoid insertChild(HtmlViewList::Iterator& iter, HtmlView* child);

private:
	LBool           m_isChildrenInline;
	LBool           m_isAnonymous;
	/** The vertical scroll position of this item. */
	LInt            m_scrollY;

	/** The horisontal scroll position of this item */
	LInt            m_scrollX;
};

}
#endif /* BlockView_H_ */

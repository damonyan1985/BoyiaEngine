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
namespace yanbo {

// block item
class BlockView : public HtmlView {
public:
    BlockView(const String& id, LBool selectable);

public:
    // layout method inline or block child
    LVoid setChildrenInline(LBool isInline);
    LBool isChildrenInline();

    // add create anonymous or normal child
    virtual LVoid addChild(HtmlView* child) LOverride;
    LVoid addChild(HtmlView* child, LBool isAnonymousBlock);

    // layer layout
    virtual LVoid layout() LOverride;
    LVoid measureBlock();
    LVoid layoutBlock(LBool relayoutChildren);
    LVoid layoutBlockChildren(LBool relayoutChildren);
    LVoid layoutInlineChildren();
    // layout block child
    LVoid layoutBlockChild(HtmlView* child, LayoutUnit& previousLogicalHeight);

    LVoid layoutPositionChild(HtmlView* child);
    LVoid makeChildrenNonInline(HtmlView* block);
    BlockView* createAnonymousBlock();

    virtual LBool isBlockView() const LOverride;
    LVoid setIsAnonymousBlock(LBool isAnonymous);
    virtual LBool isAnonymousBlock();

    LVoid setScrollPos(LInt x, LInt y);
    LInt getScrollXPos() const;
    LInt getScrollYPos() const;
    LBool canScroll() const;

    // Can scroll max height
    LInt scrollHeight() const;
    LVoid insertChild(HtmlViewList::Iterator& iter, HtmlView* child);

protected:
    virtual ~BlockView();

private:
    LBool m_isChildrenInline;
    LBool m_isAnonymous;
    /** The vertical scroll position of this item. */
    LInt m_scrollY;

    /** The horisontal scroll position of this item */
    LInt m_scrollX;
};
}
#endif /* BlockView_H_ */

#include "FlexLayout.h"

namespace yanbo
{
LVoid FlexLayout::flexRowLayout(HtmlView* view)
{
    const HtmlViewList& list = view->getChildren();
    HtmlViewList::Iterator iter = list.begin();
    HtmlViewList::Iterator iterEnd = list.end();
    LInt x = view->getStyle()->leftPadding;
    LInt y = view->getStyle()->topPadding;
    KFORMATLOG("layoutInlineBlock, child size=%d", list.count());
    for (; iter != iterEnd; ++iter)
    {
        x += (*iter)->getStyle()->leftMargin;
        (*iter)->setPos(x, y);
        KFORMATLOG("layoutInlineBlock, x=%d, y=%d", x, y);
        (*iter)->layout();
        x += (*iter)->getWidth();
        if (view->getHeight() < (*iter)->getHeight())
        {
        	view->setHeight((*iter)->getHeight());
        }
    }
}

LVoid FlexLayout::flexRowReverse(HtmlView* view)
{
    const HtmlViewList& list = view->getChildren();
    HtmlViewList::Iterator iter = list.begin();
    HtmlViewList::Iterator iterEnd = list.end();
    LInt x = view->getWidth() - view->getStyle()->rightPadding;
    LInt y = view->getStyle()->topPadding;
    KFORMATLOG("layoutInlineBlock, child size=%d", list.count());
    for (; iter != iterEnd; ++iter)
    {
        x -= (*iter)->getStyle()->rightMargin;
        (*iter)->setPos(x, y);
        KFORMATLOG("layoutInlineBlock, x=%d, y=%d", x, y);
        (*iter)->layout();
        x -= (*iter)->getWidth();
        if (view->getHeight() < (*iter)->getHeight())
        {
            view->setHeight((*iter)->getHeight());
        }
    }
}
}

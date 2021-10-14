#include "FlexLayout.h"

namespace yanbo {
// 限定: 只有子元素都是block才能使用flex布局
// 本引擎盒子与W3C的盒子模型计算略有不同，本引擎盒子设置的宽高
// width，height包含了边框+padding+内容的宽高，但不包含margin
// 而W3C中设置元素的width，height只包含了内容宽高，
// W3C真实的盒子宽高需要加上margin+border+padding+内容宽高
// 本引擎内容宽高是需要计算才能得出的
LVoid FlexLayout::flexRowLayout(HtmlView* view)
{
    const HtmlViewList& list = view->getChildren();
    HtmlViewList::Iterator iter = list.begin();
    HtmlViewList::Iterator iterEnd = list.end();
    LInt x = view->getStyle()->leftPadding;
    LInt y = view->getStyle()->topPadding;
    KFORMATLOG("layoutInlineBlock, child size=%d", list.count());
    for (; iter != iterEnd; ++iter) {
        HtmlView* child = (*iter);
        x += child->getStyle()->leftMargin;
        y += child->getStyle()->topMargin;
        child->setPos(x, y);
        KFORMATLOG("layoutInlineBlock, x=%d, y=%d", x, y);
        child->layout();
        x += child->getWidth() + child->getStyle()->rightMargin;
        if (view->getHeight() < child->getHeight() + y) {
            view->setHeight(child->getHeight() + y);
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
    for (; iter != iterEnd; ++iter) {
        HtmlView* child = (*iter);
        x -= child->getStyle()->rightMargin;
        y += child->getStyle()->topMargin;
        child->setPos(x, y);
        KFORMATLOG("layoutInlineBlock, x=%d, y=%d", x, y);
        child->layout();
        x -= child->getWidth() + child->getStyle()->leftMargin;
        if (view->getHeight() < child->getHeight() + y) {
            view->setHeight(child->getHeight() + y);
        }
    }
}
}

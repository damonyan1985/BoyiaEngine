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
    LInt x = view->getStyle()->padding().leftPadding;
    LInt deltaY = view->getStyle()->border().topWidth + view->getStyle()->padding().topPadding;
    BOYIA_LOG("layoutInlineBlock, child size=%d", list.count());
    for (; iter != iterEnd; ++iter) {
        HtmlView* child = (*iter);
        x += child->getStyle()->margin().leftMargin;
        LInt y = deltaY + child->getStyle()->margin().topMargin;
        child->setPos(x, y);
        BOYIA_LOG("layoutInlineBlock, x=%d, y=%d", x, y);
        child->layout();
        x += child->getWidth() + child->getStyle()->margin().rightMargin;
        if (view->getHeight() < child->getHeight() + y) {
            view->setHeight(child->getHeight() + y);
        }
    }
    
    // 如果style中没有设置宽度，则flex布局宽高根据元素自己的宽高来限定
    if (!view->getStyle()->width) {
        view->setWidth(x);
    }
}

LVoid FlexLayout::flexRowReverse(HtmlView* view)
{
    const HtmlViewList& list = view->getChildren();
    HtmlViewList::Iterator iter = list.begin();
    HtmlViewList::Iterator iterEnd = list.end();
    LInt x = view->getWidth() - view->getStyle()->padding().rightPadding;
    LInt deltaY = view->getStyle()->border().topWidth + view->getStyle()->padding().topPadding;
    BOYIA_LOG("layoutInlineBlock, child size=%d", list.count());
    for (; iter != iterEnd; ++iter) {
        HtmlView* child = (*iter);
        x -= child->getStyle()->margin().rightMargin;
        LInt y = deltaY + child->getStyle()->margin().topMargin;
        child->setPos(x, y);
        BOYIA_LOG("layoutInlineBlock, x=%d, y=%d", x, y);
        child->layout();
        x -= child->getWidth() + child->getStyle()->margin().leftMargin;
        if (view->getHeight() < child->getHeight() + y) {
            view->setHeight(child->getHeight() + y);
        }
    }
}
}

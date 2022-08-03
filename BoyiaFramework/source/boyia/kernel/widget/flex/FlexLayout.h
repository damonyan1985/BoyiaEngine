#ifndef FlexLayout_h
#define FlexLayout_h

#include "HtmlView.h"

namespace yanbo {
class FlexLayout {
public:
    // 从左使用行排版
    static LVoid flexRowLayout(HtmlView* view);
    // 从右使用行排版
    static LVoid flexRowReverse(HtmlView* view);
    // 等间距行排版
    static LVoid flexRowSpacebetween(HtmlView* view);
    
    // 从上到下使用列排版
    static LVoid flexColumnLayout(HtmlView* view);
    // 从下到上使用列排版
    static LVoid flexColumnReverseLayout(HtmlView* view);
    // 等间距列排版
    static LVoid flexColumnSpacebetween(HtmlView* view);
};
}

#endif

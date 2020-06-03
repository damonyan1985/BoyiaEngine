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
};
}

#endif

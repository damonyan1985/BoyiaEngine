#ifndef FlexLayout_h
#define FlexLayout_h

#include "BlockView.h"

namespace yanbo {
class FlexLayout {
public:
    // 从左使用行排版
    static LVoid flexRowLayout(BlockView* view);
    // 从右使用行排版
    static LVoid flexRowReverse(BlockView* view);
    // 等间距行排版
    static LVoid flexRowSpacebetween(BlockView* view);
    
    // 从上到下使用列排版
    static LVoid flexColumnLayout(BlockView* view);
    // 从下到上使用列排版
    static LVoid flexColumnReverseLayout(BlockView* view);
    // 等间距列排版
    static LVoid flexColumnSpacebetween(BlockView* view);
};
}

#endif

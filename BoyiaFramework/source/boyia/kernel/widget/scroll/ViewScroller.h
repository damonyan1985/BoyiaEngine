//
//  ViewScroller.h
//  widget scroller
//
//  Created by yanbo on 2021/9/22.
//

#ifndef ViewScroller_h
#define ViewScroller_h

#include "PlatformLib.h"

namespace yanbo {
class ViewScroller {
public:
    LInt scrollX() const;
    LInt scrollY() const;
    
private:
    /** The vertical scroll position of this item. */
    LInt m_scrollY;

    /** The horisontal scroll position of this item */
    LInt m_scrollX;
};
}


#endif /* ViewScroller_h */

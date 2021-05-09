/*
 * UIViewController.h
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */

#ifndef UIViewController_h
#define UIViewController_h

#include "HtmlView.h"
#include "LGraphic.h"
#include "PlatformLib.h"
#include "HashUtil.h"
#include "HashMap.h"
#include "Animation.h"

namespace yanbo {
using ScrollMap = HashMap<HashPtr, WeakPtr<Animation>>;
class UIView;
class UIViewController {
public:
    UIViewController(UIView* view);
    ~UIViewController();

public:
    // Key Event
    void onUpKey(LBool repeated);
    void onLeftKey(LBool repeated);
    void onDownKey(LBool repeated);
    void onRightKey(LBool repeated);
    void onEnterKey(LBool repeated);

    // Mouse Event
    void onMousePress(const LPoint& pt);
    void onMouseUp(const LPoint& pt);

    // Touch Event
    void onTouchDown(const LPoint& pt);
    void onTouchUp(const LPoint& pt);
    void onTouchMove(const LPoint& pt);

    LBool hitTest(const LPoint& pt);

    // pt1 手势按下时的坐标
    // pt2 手势放开时的坐标
    // velocityX x轴方向速度
    // velocityY y轴方向速度
    LVoid onFling(const LPoint& pt1,
        const LPoint& pt2,
        LReal velocityX,
        LReal velocityY);

protected:
    UIView* m_view;
    HtmlView* m_target;
    LPoint m_targetPoint;
    ScrollMap m_scrollMap;
};
}
#endif /* UIViewCONTROLLER_H_ */

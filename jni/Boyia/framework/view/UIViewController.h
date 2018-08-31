/*
 * UIViewController.h
 *
 *  Created on: 2011-7-15
 *      Author: yanbo
 */

#ifndef UIViewController_h
#define UIViewController_h

#include "PlatformLib.h"
#include "LGraphic.h"
#include "HtmlView.h"

namespace yanbo
{

class UIView;
class UIViewController
{
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

protected:
	UIView*          m_view;
	HtmlView*        m_target;
	LPoint           m_targetPoint;
};

}
#endif /* UIViewCONTROLLER_H_ */

/*
 * IViewListener.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef IViewListener_h
#define IViewListener_h

namespace yanbo
{

class IViewListener
{
public:
    virtual void onPressDown(void* view) = 0;
    virtual void onPressMove(void* view) = 0;
    virtual void onPressUp(void* view) = 0;
    virtual void onKeyDown(int keyCode, void* view) = 0;
    virtual void onKeyUp(int keyCode, void* view) = 0;
};

}
#endif /* IITEMLISTENER_H_ */

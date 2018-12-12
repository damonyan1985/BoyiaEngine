/**
 * Description: BoyiaAppStack
 *              AppStack For Boyia Programming
 * Author: Yan Bo
 * Date: 2018-10-5
 */

#ifndef BoyiaAppStack_h
#define BoyiaAppStack_h

#include "BoyiaApplication.h"

namespace boyia
{
class BoyiaAppStack
{
public:
    BoyiaAppStack();
    ~BoyiaAppStack();

    LVoid pushApp(BoyiaApplication* app);
    LVoid popApp();

private:
    Stack<BoyiaApplication*> m_appStack;
};
}

#endif

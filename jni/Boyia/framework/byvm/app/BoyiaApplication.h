/**
 * Description: BoyiaApplication
 *              Application For Boyia Programming
 * Author: Yan Bo
 * Date: 2018-10-5
 */
#ifndef BoyiaApplication_h
#define BoyiaApplication_h

#include "Stack.h"
#include "BoyiaActivity.h"

namespace boyia
{
class BoyiaApplication
{
public:
	BoyiaApplication();
    LVoid initApp();

private:
    Stack<BoyiaActivity*> m_stack;
    LVoid* m_vm;
};
}

#endif

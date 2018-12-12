/**
 * Description: BoyiaApplication
 *              Application For Boyia Programming
 * Author: Yan Bo
 * Date: 2018-10-5
 */
#ifndef BoyiaApplication_h
#define BoyiaApplication_h

#include "Stack.h"
#include "BoyiaCore.h"

namespace boyia
{
class BoyiaApplication
{
public:
	BoyiaApplication();
	~BoyiaApplication();

    LVoid resume();

private:
    Stack<BoyiaValue*> m_actStack;
    LVoid* m_vm;
};
}

#endif

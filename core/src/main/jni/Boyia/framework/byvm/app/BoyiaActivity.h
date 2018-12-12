#ifndef BoyiaActivity_h
#define BoyiaActivity_h

#include "BoyiaCore.h"

namespace boyia
{
class BoyiaActivity
{
public:
    BoyiaActivity();

    LVoid create();
    LVoid resume();
    LVoid pause();
    LVoid destroy();

private:
    BoyiaValue m_actVal;
};
}

#endif

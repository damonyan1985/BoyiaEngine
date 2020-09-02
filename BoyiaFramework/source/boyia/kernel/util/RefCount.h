#ifndef RefCount_h
#define RefCount_h

#include "PlatformLib.h"

namespace util {
class RefCount {
public:
    RefCount();
    ~RefCount();

    LVoid ref();
    LVoid deref();
    LInt shareCount() const;
    LInt weakCount() const;

    LVoid addWeak();
    LVoid detchWeak();
    LVoid release();

private:
    LInt m_shareCount;
    LInt m_weakCount;
};
}

#endif
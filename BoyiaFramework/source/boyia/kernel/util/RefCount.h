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

    LVoid attachWeak();
    LVoid detchWeak();
    LVoid release();

private:
    // 此处引用计数不是原子操作
    // 简化程序开发，智能指针只在单线程上进行使用
    LInt m_shareCount;
    LInt m_weakCount;
};
}

#endif
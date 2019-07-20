/*
 * MiniMutex.cpp
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#ifndef MiniTaskBase_h
#define MiniTaskBase_h
#include "BoyiaRef.h"

namespace yanbo {

class MiniTaskBase : public BoyiaRef {
public:
    virtual ~MiniTaskBase();

public:
    virtual void execute() = 0;
};
}

#endif

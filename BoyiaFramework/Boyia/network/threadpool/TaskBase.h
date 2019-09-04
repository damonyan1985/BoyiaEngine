/*
 * TaskBase.h
 *
 *  Created on: 2015-11-20
 *      Author: yanbo
 */
#ifndef TaskBase_h
#define TaskBase_h
#include "BoyiaRef.h"

namespace yanbo {

class TaskBase : public BoyiaRef {
public:
    virtual ~TaskBase();

public:
    virtual void execute() = 0;
};
}

#endif

#ifndef BoyiaBase_h
#define BoyiaBase_h

#include "BoyiaCore.h"
#include "IViewListener.h"
#include "KVector.h"
#include "BoyiaRuntime.h"

namespace boyia {

class BoyiaBase {
public:
    BoyiaBase(BoyiaRuntime* runtime);
    virtual LVoid release();

    BoyiaRuntime* runtime() const;
    LVoid setGcFlag(LInt flag);
    LInt gcFlag() const;

protected:
    virtual ~BoyiaBase();
    
    BoyiaRuntime* m_runtime;
    LInt m_gcFlag;
};
}

#endif

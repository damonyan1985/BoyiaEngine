#ifndef IRenderEngine_h
#define IRenderEngine_h

#include "RenderLayer.h"

namespace yanbo {
// 根据平台来适配    
class IRenderEngine {
public:
    static IRenderEngine* create();

    virtual ~IRenderEngine() {};
    virtual LVoid init() = 0;
    virtual LVoid reset() = 0;
    virtual LVoid render(RenderLayer* layer) = 0;
};
}

#endif


//
//  RenderEngineIOS.h
//  core
//
//  Created by yanbo on 2021/8/27.
//

#ifndef RenderEngineIOS_h
#define RenderEngineIOS_h

#include "IRenderEngine.h"

@class IOSRenderer;

namespace yanbo {
// TODO
class RenderEngineIOS : public IRenderEngine {
public:
    RenderEngineIOS();
    virtual ~RenderEngineIOS();
    virtual LVoid init();
    virtual LVoid reset();
    virtual LVoid render(RenderLayer* layer);
    
    LVoid setContextIOS(IOSRenderer* metalLayer);
    
    LVoid renderRect(RenderCommand* cmd);
    LVoid renderImage(RenderCommand* cmd);
    LVoid renderText(RenderCommand* cmd);
    
private:
    IOSRenderer* m_renderer;
};
}
#endif /* RenderEngineIOS_h */

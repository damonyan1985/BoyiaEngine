//
//  RenderEngineIOS.h
//  core
//
//  Created by yanbo on 2021/8/27.
//

#ifndef RenderEngineIOS_h
#define RenderEngineIOS_h

#include "IRenderEngine.h"
#include "ShaderType.h"

@class IOSRenderer;

namespace yanbo {
LVoid screenToMetalPoint(int x, int y, float* metalX, float* metalY);
// TODO
class RenderEngineIOS;
typedef LVoid (RenderEngineIOS::*RenderFunction)(RenderCommand* cmd);
class RenderEngineIOS : public IRenderEngine {
public:
    RenderEngineIOS();
    virtual ~RenderEngineIOS();

    virtual LVoid init();
    virtual LVoid reset();
    virtual LVoid render(RenderLayer* layer);
    
    LVoid setContextIOS(IOSRenderer* renderer);
    
    LVoid renderImpl(RenderLayer* layer);
    LVoid renderRect(RenderCommand* cmd);
    LVoid renderImage(RenderCommand* cmd);
    LVoid renderText(RenderCommand* cmd);
    
    LVoid renderRectEx(RenderCommand* cmd);
    
    LVoid setBuffer();
    
private:
    KVector<VertexAttributes> m_vertexs;
    IOSRenderer* m_renderer;
    RenderFunction m_functions[5];
};
}
#endif /* RenderEngineIOS_h */

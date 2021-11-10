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
LVoid screenToMetalPoint(float x, float y, float* metalX, float* metalY);
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
    LVoid renderRoundRect(RenderCommand* cmd);
    
    // Just for test
    LVoid renderRoundRectEx(RenderCommand* cmd);
    LVoid renderRectEx(RenderCommand* cmd);
    
    LVoid setBuffer();
    
    LVoid appendUniforms(LInt type);
    const KVector<Uniforms>& uniforms() const;
        
    IOSRenderer* iosRenderer() const;
    
private:
    KVector<VertexAttributes> m_vertexs;
    KVector<Uniforms> m_uniforms;
    
    IOSRenderer* m_renderer;
    RenderFunction m_functions[6];
};
}
#endif /* RenderEngineIOS_h */

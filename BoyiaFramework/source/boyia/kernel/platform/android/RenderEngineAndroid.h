#ifndef RenderEngineAndroid_h
#define RenderEngineAndroid_h

#include "IRenderEngine.h"
#include "GLContext.h"
#include "GLPainter.h"
#include "KList.h"
#include "BoyiaPtr.h"

namespace yanbo {

typedef KList<BoyiaPtr<GLPainter>> ListPainter;

// Android平台渲染引擎
class RenderEngineAndroid;
typedef LVoid (RenderEngineAndroid::*RenderFunction)(RenderCommand* cmd);
class RenderEngineAndroid : public IRenderEngine {
public:
    RenderEngineAndroid();
    virtual ~RenderEngineAndroid();
    virtual LVoid init();
    virtual LVoid reset();
    virtual LVoid render(RenderLayer* layer);

private:
    LVoid renderImpl(RenderLayer* layer);
    LVoid renderRect(RenderCommand* cmd);
    LVoid renderImage(RenderCommand* cmd);
    LVoid renderText(RenderCommand* cmd);
    LVoid renderRoundRect(RenderCommand* cmd);
    LVoid renderVideo(RenderCommand* cmd);
    LVoid renderRoundImage(RenderCommand* cmd);
    LVoid renderPlatform(RenderCommand* cmd);

    LVoid renderSubmit();

    RenderFunction m_functions[7];


    util::GLContext m_context;
    ListPainter m_painters;
};
}
#endif
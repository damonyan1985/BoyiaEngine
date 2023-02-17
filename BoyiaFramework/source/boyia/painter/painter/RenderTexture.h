#ifndef RenderTexture_h
#define RenderTexture_h

#include "TextureCache.h"
#ifdef OPENGLES_3
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

namespace yanbo {
// 利用FBO做离屏渲染
class RenderTexture {
public:
    RenderTexture();
    ~RenderTexture();

    LVoid initFBO(LInt width, LInt height);
    LVoid bind();
    LVoid unbind();

private:
    Texture* m_tex;
    GLuint m_fbo;
    GLuint m_rbo;
};
} // namespace yanbo
#endif

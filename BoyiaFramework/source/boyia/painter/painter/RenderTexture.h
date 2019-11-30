#ifndef RenderTexture_h
#define RenderTexture_h

#include "TextureCache.h"
#include <GLES3/gl3.h>

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

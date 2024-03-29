#ifndef GLProgram_h
#define GLProgram_h


#include "PlatformLib.h"

#ifdef OPENGLES_3
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

namespace yanbo {
class GLProgram {
public:
    enum {
        PROGRAM_ATTRIB_POSITION = 0,
        PROGRAM_ATTRIB_COLOR,
        PROGRAM_ATTRIB_TEX_COORD,
    };
    GLProgram();
    ~GLProgram();
    void initShader();
    void initAttribute(GLuint program);
    void use(int type);

    bool available();

    //GLuint position();
    //GLuint color();
    //GLuint texCoord();
    GLuint texFlag();
    GLuint matrix();
    GLuint sampler2D();

    GLuint videoMatrix();
    GLuint videoSTMatrix();
    GLuint videoSampler2D();

    GLuint radius();
    GLuint rect();
    GLuint ratio();

    GLuint m_videoPositionHandle;
    GLuint m_videoColorHandle;
    GLuint m_videoTextureHandle;

private:
    GLuint m_normalPrgm;
    GLuint m_videoPrgm;
    //GLuint m_aPositionHandle;
    //GLuint m_aColorHandle;
    GLuint m_muMVPMatrixHandle;
    GLuint m_uShapeTypeHandle;
    GLuint m_uRadius;
    GLuint m_uRect;
    GLuint m_uRatio;

    //GLuint m_aTextureCoord;
    GLuint m_uSampler2D;
    // Video Handle
    GLuint m_videoMVPMatrix;
    GLuint m_videoSTMatrix;
    GLuint m_videoSampler2D;
};
} // namespace yanbo
#endif

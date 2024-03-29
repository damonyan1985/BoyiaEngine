#ifndef GLPainter_h
#define GLPainter_h

#include "BoyiaRenderer.h"
#include "KListMap.h"
#include "KVector.h"
#include "LGdi.h"
#include "TextureCache.h"
#include <jni.h>

#ifdef OPENGLES_3
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

using namespace util;

namespace yanbo {
class RotateInfo {
public:
    RotateInfo();

    float rx;
    float ry;
    float rz;
};

class RoundRectInfo {
public:
    RoundRectInfo();
    RoundRectInfo(const RoundRectInfo& info);

    float left;
    float top;
    float right;
    float bottom;
    // 圆角半径设置
    float topLeftRadius;
    float topRightRadius;
    float bottomRightRadius; 
    float bottomLeftRadius;
};

class PaintCommand {
public:
    PaintCommand();
    ~PaintCommand();

    LInt top;

    Quad quad;
    RotateInfo rotate;
    GLuint texId;
    LInt type;
    float* matrix;

    RoundRectInfo round;
};

class BatchCommand {
public:
    BatchCommand();

    LInt type;
    GLuint texId;
    LInt size;
    float* matrix;

    RoundRectInfo round;
};

class BatchCommandBuffer {
public:
    BatchCommandBuffer();

    bool sameMaterial(const PaintCommand& cmd);
    LVoid addBatchCommand();
    LVoid addBatchCommand(const PaintCommand& cmd);
    LVoid reset();

    BatchCommand buffer[INDEX_SIZE];
    LInt size;
};

class GLPainter : public BoyiaRef {
public:
    enum ShapeType {
        EShapeNone = 0,
        EShapeLine,
        EShapeRect,
        EShapeRoundRect,
        EShapeString,
        EShapeImage,
        EShapeRoundImage,
        EShapeExternal,
    };

public:
    GLPainter();
    virtual ~GLPainter();

    static void bindVBO();
    static void unbindVBO();
    static void init();
    static void paintCommand();
    static void reset();

public:
    void setColor(const LColor& color);

    void setRect(const LRect& rect);
    void setRoundRect(
        const LRect& rect, 
        LInt topLeftRadius, 
        LInt topRightRadius, 
        LInt bottomRightRadius, 
        LInt bottomLeftRadius);

    void setLine(const LPoint& p1, const LPoint& p2);
    void setImage(Texture* tex, const LRect& rect);
    void setImage(Texture* tex, const LRect& rect, const LRect& clipRect,
        LInt topLeftRadius, 
        LInt topRightRadius, 
        LInt bottomRightRadius, 
        LInt bottomLeftRadius);
    void setImage(Texture* tex, const LRect& rect, const LRect& clipRect);
    void setExternal(Texture* tex, const LRect& rect);

    void setScale(float scale);
    void appendToBuffer();
    float* stMatrix() const;

protected:
    void setTexture(Texture* tex, const LRect& rect, const LRect& clipRect);

    PaintCommand m_cmd;
    float m_scale;
    float* m_stMatrix;
    static BatchCommandBuffer s_buffer;
};
} // namespace yanbo
#endif

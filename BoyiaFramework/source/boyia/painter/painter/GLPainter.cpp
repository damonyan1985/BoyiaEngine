#include "GLPainter.h"
#include "BoyiaPainterEnv.h"
#include "MatrixState.h"
#include "SalLog.h"
#include "ShaderUtil.h"
#include "StringUtils.h"

namespace yanbo {
BatchCommandBuffer GLPainter::s_buffer;

RotateInfo::RotateInfo()
    : rx(0)
    , ry(0)
    , rz(0)
{
}

PaintCommand::PaintCommand()
    : texId(0)
    , type(GLPainter::EShapeNone)
    , matrix(NULL)
{
}

BatchCommand::BatchCommand()
    : texId(0)
    , type(GLPainter::EShapeNone)
    , matrix(NULL)
{
}

BatchCommandBuffer::BatchCommandBuffer()
    : size(0)
{
}

bool BatchCommandBuffer::sameMaterial(GLuint texId)
{
    KFORMATLOG("BatchCommandBuffer::sameMaterial prevTexid=%u texId=%u", buffer[size - 1].texId, texId);
    return size > 0 && buffer[size - 1].texId == texId;
    //return false;
}

LVoid BatchCommandBuffer::addBatchCommand()
{
    if (size > 0) {
        buffer[size - 1].size++;
    }
}

LVoid BatchCommandBuffer::addBatchCommand(const PaintCommand& cmd)
{
    buffer[size].texId = cmd.texId;
    buffer[size].type = cmd.type;
    buffer[size].matrix = cmd.matrix;
    buffer[size].size = 1;
    size++;
}

LVoid BatchCommandBuffer::reset()
{
    size = 0;
}

GLPainter::GLPainter()
    : m_scale(1)
    , m_stMatrix(NULL)
{
}

GLPainter::~GLPainter()
{
    if (m_stMatrix) {
        delete m_stMatrix;
    }
}

void GLPainter::setLine(const LPoint& p1, const LPoint& p2)
{
}

void GLPainter::setRect(const LRect& rect)
{
    m_cmd.type = EShapeRect;
    Quad& quad = m_cmd.quad;
    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &quad.topLeft.vec3D.x, &quad.topLeft.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &quad.topRight.vec3D.x, &quad.topRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &quad.bottomRight.vec3D.x, &quad.bottomRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &quad.bottomLeft.vec3D.x, &quad.bottomLeft.vec3D.y);
}

void GLPainter::setImage(Texture* tex, const LRect& rect)
{
    m_cmd.type = EShapeImage;
    setTexture(tex, rect, rect);
}

void GLPainter::setImage(Texture* tex, const LRect& rect, const LRect& clipRect)
{
    m_cmd.type = EShapeImage;

    // LInt left = rect.iTopLeft.iX > clipRect.iTopLeft.iX ? rect.iTopLeft.iX : clipRect.iTopLeft.iX;
    // LInt top = rect.iTopLeft.iY > clipRect.iTopLeft.iY ? rect.iTopLeft.iY : clipRect.iTopLeft.iY;
    // LInt right = rect.iBottomRight.iX > clipRect.iBottomRight.iX ? clipRect.iBottomRight.iX : rect.iBottomRight.iX;
    // LInt bottom = rect.iBottomRight.iY > clipRect.iBottomRight.iY ? clipRect.iBottomRight.iY : rect.iBottomRight.iY;
    // LRect newRect(left, top, right - left, bottom - top);

    setTexture(tex, rect, clipRect);
}

float* GLPainter::stMatrix() const
{
    return m_stMatrix;
}

void GLPainter::setExternal(Texture* tex, const LRect& rect)
{
    if (!m_stMatrix) {
        m_stMatrix = new float[16];
    }

    m_cmd.top = rect.iTopLeft.iY;
    m_cmd.type = EShapeExternal;
    m_cmd.texId = tex->texId;
    m_cmd.matrix = m_stMatrix;

    Quad& quad = m_cmd.quad;
    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &quad.bottomLeft.vec3D.x, &quad.bottomLeft.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &quad.bottomRight.vec3D.x, &quad.bottomRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &quad.topRight.vec3D.x, &quad.topRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &quad.topLeft.vec3D.x, &quad.topLeft.vec3D.y);

    // 纹理坐标
    quad.bottomLeft.texCoord.u = 0.0f;
    quad.bottomLeft.texCoord.v = 0.0f; // 0 0

    quad.bottomRight.texCoord.u = 1.0f;
    quad.bottomRight.texCoord.v = 0.0f; // 1 0

    quad.topRight.texCoord.u = 1.0f;
    quad.topRight.texCoord.v = 1.0f;

    quad.topLeft.texCoord.u = 0.0f;
    quad.topLeft.texCoord.v = 1.0f;
}

void GLPainter::setTexture(Texture* tex, const LRect& rect, const LRect& clipRect)
{
    m_cmd.texId = tex->texId;
    KFORMATLOG("texSize width=%d. height=%d", tex->width, tex->height);
    KFORMATLOG("LRect width=%d. height=%d", rect.GetWidth(), rect.GetHeight());

    Quad& quad = m_cmd.quad;
    // ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &quad.bottomLeft.vec3D.x, &quad.bottomLeft.vec3D.y);

    // ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &quad.bottomRight.vec3D.x, &quad.bottomRight.vec3D.y);

    // ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &quad.topRight.vec3D.x, &quad.topRight.vec3D.y);

    // ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &quad.topLeft.vec3D.x, &quad.topLeft.vec3D.y);

    float left = clipRect.iTopLeft.iX <= rect.iTopLeft.iX ?
        rect.iTopLeft.iX : clipRect.iTopLeft.iX;
    float top = clipRect.iTopLeft.iY <= rect.iTopLeft.iY ?
        rect.iTopLeft.iY : clipRect.iTopLeft.iY;

    float right = clipRect.iBottomRight.iX >= rect.iBottomRight.iX ?
        rect.iBottomRight.iX : clipRect.iBottomRight.iX;
    float bottom = clipRect.iBottomRight.iY >= rect.iBottomRight.iY ?
        rect.iBottomRight.iY : clipRect.iBottomRight.iY;

    ShaderUtil::screenToGlPoint(left, bottom, &quad.bottomLeft.vec3D.x, &quad.bottomLeft.vec3D.y);

    ShaderUtil::screenToGlPoint(right, bottom, &quad.bottomRight.vec3D.x, &quad.bottomRight.vec3D.y);

    ShaderUtil::screenToGlPoint(right, top, &quad.topRight.vec3D.x, &quad.topRight.vec3D.y);

    ShaderUtil::screenToGlPoint(left, top, &quad.topLeft.vec3D.x, &quad.topLeft.vec3D.y);      


    float texL = clipRect.iTopLeft.iX <= rect.iTopLeft.iX ?
        0 : ((float)(clipRect.iTopLeft.iX - rect.iTopLeft.iX)) / clipRect.GetWidth();
    float texT = clipRect.iTopLeft.iY <= rect.iTopLeft.iY ?
        0 : ((float)(clipRect.iTopLeft.iY - rect.iTopLeft.iY)) / clipRect.GetHeight();

    float texR = clipRect.iBottomRight.iX >= rect.iBottomRight.iX ?
        1.0f : ((float)(clipRect.iBottomRight.iX - clipRect.iTopLeft.iX)) / clipRect.GetWidth();
    float texB = clipRect.iBottomRight.iY >= rect.iBottomRight.iY ?
        1.0f : ((float)(clipRect.iBottomRight.iY - rect.iTopLeft.iY)) / clipRect.GetHeight();
    // 纹理坐标
    // quad.bottomLeft.texCoord.u = 0.0f;
    // quad.bottomLeft.texCoord.v = 1.0f;

    // quad.bottomRight.texCoord.u = 1.0f;
    // quad.bottomRight.texCoord.v = 1.0f;

    // quad.topRight.texCoord.u = 1.0f;
    // quad.topRight.texCoord.v = 0.0f;

    // quad.topLeft.texCoord.u = 0.0f;
    // quad.topLeft.texCoord.v = 0.0f;

    quad.bottomLeft.texCoord.u = texL;
    quad.bottomLeft.texCoord.v = texB;

    quad.bottomRight.texCoord.u = texR;
    quad.bottomRight.texCoord.v = texB;

    quad.topRight.texCoord.u = texR;
    quad.topRight.texCoord.v = texT;

    quad.topLeft.texCoord.u = texL;
    quad.topLeft.texCoord.v = texT;
}

void GLPainter::reset()
{
    s_buffer.reset();
}

void GLPainter::paintCommand()
{
    if (!BoyiaPainterEnv::instance()->available()) {
        return;
    }

    LInt drawQuadIndex = 0;

    for (LInt i = 0; i < s_buffer.size; i++) {
        KLOG("BaseShape::drawSelf()");

        MatrixState::pushMatrix();
        // 制定使用某套shader程序
        BoyiaPainterEnv::instance()->program()->use(s_buffer.buffer[i].type);
        KLOG("BaseShape::drawSelf()1");
        // 初始化变换矩阵
        KLOG("BaseShape::drawSelf()2");
        // 设置沿Z轴正向位移1
        //MatrixState::translate(0.0f, 0.0f, 0.0f);
        KLOG("BaseShape::drawSelf()3");
        // 设置绕y轴旋转
        MatrixState::rotate(0, 0, 1, 0);
        KLOG("BaseShape::drawSelf()4");
        // 设置绕z轴旋转
        MatrixState::rotate(0, 1, 0, 0);

        KLOG("BaseShape::drawSelf()5");

        switch (s_buffer.buffer[i].type) {
        case EShapeExternal: {
            glUniformMatrix4fv(BoyiaPainterEnv::instance()->program()->videoMatrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            if (s_buffer.buffer[i].matrix) {
                glUniformMatrix4fv(BoyiaPainterEnv::instance()->program()->videoSTMatrix(), 1, GL_FALSE, s_buffer.buffer[i].matrix);
            }

            //KFORMATLOG("m_shapeType == EShapeVideo error=%d", glGetError());
            glUniform1i(BoyiaPainterEnv::instance()->program()->videoSampler2D(), 0);
            // 绑定纹理
            glActiveTexture(GL_TEXTURE0);
            // Set GL_TEXTURE_EXTERNAL_OES
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, s_buffer.buffer[i].texId);
        } break;
        case EShapeImage: {
            glUniformMatrix4fv(BoyiaPainterEnv::instance()->program()->matrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            glUniform1i(BoyiaPainterEnv::instance()->program()->texFlag(), 1);

            glUniform1i(BoyiaPainterEnv::instance()->program()->sampler2D(), 0);
            //绑定纹理
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, s_buffer.buffer[i].texId);

            KFORMATLOG("GLPainter::paintCommand texSize=%d", s_buffer.buffer[i].size);
        } break;
        case EShapeRect: {
            glUniformMatrix4fv(BoyiaPainterEnv::instance()->program()->matrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            glUniform1i(BoyiaPainterEnv::instance()->program()->texFlag(), 0);
            KFORMATLOG("GLPainter::paintCommand rectSize=%d", s_buffer.buffer[i].size);
        } break;
        }

        // 绘制矩形
        glDrawElements(GL_TRIANGLES, s_buffer.buffer[i].size * 6, GL_UNSIGNED_SHORT, (GLvoid*)(drawQuadIndex * 6 * sizeof(GLushort)));
        MatrixState::popMatrix();
        drawQuadIndex += s_buffer.buffer[i].size;
    }
}

void GLPainter::appendToBuffer()
{
    BoyiaPainterEnv::instance()->appendQuad(m_cmd.quad);

    if (s_buffer.sameMaterial(m_cmd.texId)) {
        s_buffer.addBatchCommand();
    } else {
        s_buffer.addBatchCommand(m_cmd);
    }
}

void GLPainter::setColor(const LColor& color)
{
    m_cmd.quad.topLeft.color.set(color);
    m_cmd.quad.topRight.color.set(color);
    m_cmd.quad.bottomRight.color.set(color);
    m_cmd.quad.bottomLeft.color.set(color);
}

void GLPainter::setScale(float scale)
{
    m_scale = scale;
}

void GLPainter::init()
{
    BoyiaPainterEnv::instance()->init();
}

void GLPainter::bindVBO()
{
    BoyiaPainterEnv::instance()->bindVBO();
}

void GLPainter::unbindVBO()
{
    BoyiaPainterEnv::instance()->unbindVBO();
}

} // namespace yanbo
#include "GLPainter.h"
#include "BoyiaPainterEnv.h"
#include "MatrixState.h"
#include "SalLog.h"
#include "ShaderUtil.h"
#include "StringUtils.h"
#include "PixelRatio.h"

namespace yanbo {
BatchCommandBuffer GLPainter::s_buffer;

RotateInfo::RotateInfo()
    : rx(0)
    , ry(0)
    , rz(0)
{
}

RoundRectInfo::RoundRectInfo()
    : topLeftRadius(0)
    , topRightRadius(0)
    , bottomRightRadius(0)
    , bottomLeftRadius(0)
    , left(0)
    , top(0)
    , width(0)
    , height(0)
{
}

RoundRectInfo::RoundRectInfo(const RoundRectInfo& info)
    : topLeftRadius(info.topLeftRadius)
    , topRightRadius(info.topRightRadius)
    , bottomRightRadius(info.bottomRightRadius)
    , bottomLeftRadius(info.bottomLeftRadius)
    , left(info.left)
    , top(info.top)
    , width(info.width)
    , height(info.height)
{
}

PaintCommand::PaintCommand()
    : texId(0)
    , type(GLPainter::EShapeNone)
    , matrix(NULL)
{
}

PaintCommand::~PaintCommand()
{
    // if (round) {
    //     delete round;
    // }
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

bool BatchCommandBuffer::sameMaterial(const PaintCommand& cmd)
{
    KFORMATLOG("BatchCommandBuffer::sameMaterial prevTexid=%u texId=%u", buffer[size - 1].texId, cmd.texId);
    return size > 0 
        && buffer[size - 1].texId == cmd.texId 
        && (buffer[size - 1].type != GLPainter::EShapeRoundRect && cmd.type != GLPainter::EShapeRoundRect);
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

    // 圆角半径设置
    buffer[size].round = cmd.round;
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

void GLPainter::setRoundRect(
        const LRect& rect, 
        LInt topLeftRadius, 
        LInt topRightRadius, 
        LInt bottomRightRadius, 
        LInt bottomLeftRadius)
{
    m_cmd.type = EShapeRoundRect;
    Quad& quad = m_cmd.quad;

    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &quad.topLeft.vec3D.x, &quad.topLeft.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &quad.topRight.vec3D.x, &quad.topRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &quad.bottomRight.vec3D.x, &quad.bottomRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &quad.bottomLeft.vec3D.x, &quad.bottomLeft.vec3D.y);

    m_cmd.round.left = quad.topLeft.vec3D.x;
    m_cmd.round.top = quad.topLeft.vec3D.y;
    m_cmd.round.width = quad.bottomRight.vec3D.x - quad.topLeft.vec3D.x;
    m_cmd.round.height = quad.bottomRight.vec3D.y - quad.topLeft.vec3D.y;

    m_cmd.round.topLeftRadius = ShaderUtil::screenToGlWidth(topLeftRadius);
    m_cmd.round.topRightRadius = ShaderUtil::screenToGlWidth(topRightRadius);
    m_cmd.round.bottomRightRadius = ShaderUtil::screenToGlWidth(bottomRightRadius);
    m_cmd.round.bottomLeftRadius = ShaderUtil::screenToGlWidth(bottomLeftRadius);
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

// rect为纹理显示的范围
// clipRect为需要裁剪的范围
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
        0 : (((float)(clipRect.iTopLeft.iX - rect.iTopLeft.iX)) / rect.GetWidth());
    float texT = clipRect.iTopLeft.iY <= rect.iTopLeft.iY ?
        0 : (((float)(clipRect.iTopLeft.iY - rect.iTopLeft.iY)) / rect.GetHeight());

    float texR = clipRect.iBottomRight.iX >= rect.iBottomRight.iX ?
        1.0f : (((float)(clipRect.iBottomRight.iX - rect.iTopLeft.iX)) / rect.GetWidth());
    float texB = clipRect.iBottomRight.iY >= rect.iBottomRight.iY ?
        1.0f : (((float)(clipRect.iBottomRight.iY - rect.iTopLeft.iY)) / rect.GetHeight());


    // quad.bottomLeft.texCoord.u = 0.5f;
    // quad.bottomLeft.texCoord.v = 1.0f;

    // quad.bottomRight.texCoord.u = 1.0f;
    // quad.bottomRight.texCoord.v = 1.0f;

    // quad.topRight.texCoord.u = 1.0f;
    // quad.topRight.texCoord.v = 0.5f;

    // quad.topLeft.texCoord.u = 0.5f;
    // quad.topLeft.texCoord.v = 0.5f;

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

    GLProgram* program = BoyiaPainterEnv::instance()->program();
    LInt drawQuadIndex = 0;
    for (LInt i = 0; i < s_buffer.size; i++) {
        KLOG("BaseShape::drawSelf()");

        MatrixState::pushMatrix();
        // 制定使用某套shader程序
        program->use(s_buffer.buffer[i].type);
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

        BatchCommand& cmd = s_buffer.buffer[i];
        switch (cmd.type) {
        case EShapeExternal: {
            glUniformMatrix4fv(program->videoMatrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            if (cmd.matrix) {
                glUniformMatrix4fv(program->videoSTMatrix(), 1, GL_FALSE, cmd.matrix);
            }

            //KFORMATLOG("m_shapeType == EShapeVideo error=%d", glGetError());
            glUniform1i(program->videoSampler2D(), 0);
            // 绑定纹理
            glActiveTexture(GL_TEXTURE0);
            // Set GL_TEXTURE_EXTERNAL_OES
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, cmd.texId);
        } break;
        case EShapeImage: {
            glUniformMatrix4fv(program->matrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            glUniform1i(program->texFlag(), 1);

            glUniform1i(program->sampler2D(), 0);
            //绑定纹理
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cmd.texId);

            KFORMATLOG("GLPainter::paintCommand texSize=%d", cmd.size);
        } break;
        case EShapeRect: {
            glUniformMatrix4fv(program->matrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            glUniform1i(program->texFlag(), 0);
            KFORMATLOG("GLPainter::paintCommand rectSize=%d", cmd.size);
        } break;
        case EShapeRoundRect: {
            glUniformMatrix4fv(program->matrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
            glUniform1i(program->texFlag(), 2);
            glUniform4f(program->radius(),
                cmd.round.topLeftRadius,
                cmd.round.topRightRadius,
                cmd.round.bottomRightRadius,
                cmd.round.bottomLeftRadius);
            glUniform4f(program->rect(), 
                cmd.round.left,
                cmd.round.top,
                cmd.round.width,
                cmd.round.height);
            glUniform1f(program->ratio(), (PixelRatio::logicWidth() * 1.0f) / PixelRatio::logicHeight());
        } break;
        }

        // 绘制矩形
        glDrawElements(GL_TRIANGLES, cmd.size * 6, GL_UNSIGNED_SHORT, (GLvoid*)(drawQuadIndex * 6 * sizeof(GLushort)));
        MatrixState::popMatrix();
        drawQuadIndex += cmd.size;
    }
}

void GLPainter::appendToBuffer()
{
    BoyiaPainterEnv::instance()->appendQuad(m_cmd.quad);

    if (s_buffer.sameMaterial(m_cmd)) {
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
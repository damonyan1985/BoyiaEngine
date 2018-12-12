#include "GLPainter.h"
#include "ShaderUtil.h"
#include "MatrixState.h"
#include "SalLog.h"
#include "StringUtils.h"

namespace yanbo
{
GLProgram* GLPainter::s_program = NULL;
MiniRenderer* GLPainter::s_renderer = NULL;
int GLPainter::s_drawQuadIndex = 0;
#define GL_TEXTURE_EXTERNAL_OES 0x8D65

RotateInfo::RotateInfo()
    : rx(0), ry(0), rz(0)
{
}

PaintCommand::PaintCommand()
    : texId(0)
    , type(GLPainter::EShapeNone)
{
}

GLPainter::GLPainter()
    : m_scale(1)
    , m_stMatrix(NULL)
{
	init();
}

GLPainter::~GLPainter()
{
	if (m_stMatrix)
	{
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

void GLPainter::setImage(MiniTexture* tex, const LRect& rect)
{
	//m_shapeType = EShapeImage;
	m_cmd.type = EShapeImage;
	setTexture(tex, rect);
}

float* GLPainter::stMatrix() const
{
    return m_stMatrix;
}

void GLPainter::setVideo(MiniTexture* tex, const LRect& rect)
{
	if (!m_stMatrix)
	{
		m_stMatrix = new float[16];
	}

	m_cmd.type = EShapeVideo;
	m_cmd.texId = tex->texId;

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

void GLPainter::setTexture(MiniTexture* tex, const LRect& rect)
{
	m_cmd.texId = tex->texId;
	KFORMATLOG("texSize width=%d. height=%d", tex->width, tex->height);
	KFORMATLOG("LRect width=%d. height=%d", rect.GetWidth(), rect.GetHeight());

	Quad& quad = m_cmd.quad;
    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &quad.bottomLeft.vec3D.x, &quad.bottomLeft.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &quad.bottomRight.vec3D.x, &quad.bottomRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &quad.topRight.vec3D.x, &quad.topRight.vec3D.y);

    ShaderUtil::screenToGlPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &quad.topLeft.vec3D.x, &quad.topLeft.vec3D.y);

    // 纹理坐标
    quad.bottomLeft.texCoord.u = 0.0f;
    quad.bottomLeft.texCoord.v = 1.0f;

    quad.bottomRight.texCoord.u = 1.0f;
    quad.bottomRight.texCoord.v = 1.0f;

    quad.topRight.texCoord.u = 1.0f;
    quad.topRight.texCoord.v = 0.0f;

    quad.topLeft.texCoord.u = 0.0f;
    quad.topLeft.texCoord.v = 0.0f;
}

void GLPainter::appendToBuffer()
{
	s_renderer->appendQuad(m_cmd.quad);
}

void GLPainter::setColor(const LRgb& color)
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
	//MiniTextureCache::getInst()->clear();
	if (s_program)
	{
        delete s_program;
	}

	s_program = new GLProgram();
	s_program->initShader();

	if (s_renderer)
	{
		delete s_renderer;
	}

	s_renderer = new MiniRenderer();
	s_renderer->setupIndices();
	s_renderer->createVBO();
}

void GLPainter::bindVBO()
{
	s_renderer->bind();
}

void GLPainter::unbindVBO()
{
	s_renderer->unbind();
	s_drawQuadIndex = 0;
}

void GLPainter::paint()
{
	KLOG("BaseShape::drawSelf()");
    if (s_program && s_program->available())
    {
        MatrixState::pushMatrix();
		 // 制定使用某套shader程序
    	s_program->use(m_cmd.type);
		KLOG("BaseShape::drawSelf()1");
		 // 初始化变换矩阵
	    KLOG("BaseShape::drawSelf()2");
		 // 设置沿Z轴正向位移1
		//MatrixState::translate(0.0f, 0.0f, 0.0f);
		KLOG("BaseShape::drawSelf()3");
		 // 设置绕y轴旋转
		MatrixState::rotate(m_cmd.rotate.ry, 0, 1, 0);
		KLOG("BaseShape::drawSelf()4");
		// 设置绕z轴旋转
		MatrixState::rotate(m_cmd.rotate.rx, 1, 0, 0);

		KLOG("BaseShape::drawSelf()5");
		 // 将最终变换矩阵传入shader程序
		if (m_cmd.type == EShapeVideo)
		{
			glUniformMatrix4fv(s_program->videoMatrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
			if (m_stMatrix)
			{
				glUniformMatrix4fv(s_program->videoSTMatrix(), 1, GL_FALSE, m_stMatrix);
			}
		}
		else
		{
			glUniformMatrix4fv(s_program->matrix(), 1, GL_FALSE, MatrixState::getFinalMatrix()->getBuffer());
		}
		 // 传入是否为纹理
		KLOG("BaseShape::drawSelf()6");
		//bool hasTex = m_shapeType == EShapeImage || m_shapeType == EShapeVideo;
		if (m_cmd.type != EShapeVideo)
		{
			glUniform1i(s_program->texFlag(), m_cmd.type == EShapeImage ? 1 : 0);
		}

		KLOG("BaseShape::drawSelf()7");

		 if (m_cmd.type == EShapeImage)
		 {
		     glUniform1i(s_program->sampler2D(), 0);
		     //绑定纹理
		     glActiveTexture(GL_TEXTURE0);
		     glBindTexture(GL_TEXTURE_2D, m_cmd.texId);
		 }
		 else if (m_cmd.type == EShapeVideo)
		 {
			 //KFORMATLOG("m_shapeType == EShapeVideo error=%d", glGetError());
			 glUniform1i(s_program->videoSampler2D(), 0);
			 // 绑定纹理
			 glActiveTexture(GL_TEXTURE0);
			 // 为啥要用GL_TEXTURE_2D而不是GL_TEXTURE_EXTERNAL_OES，
			 // 作者表示自己也很晕
			 // 可能出错信息会驱动GLConsumer去创建EGLImage吧
			 // 纯JAVA实现的情况会不同，蛋疼，艹
			 glBindTexture(GL_TEXTURE_2D, m_cmd.texId);

			 KFORMATLOG("m_shapeType == EShapeVideo error=%d and texId=%d", glGetError(), m_cmd.texId);
		 }

		 // 绘制矩形
		 glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (GLvoid*) (s_drawQuadIndex * 6 * sizeof(GLushort)));
		 MatrixState::popMatrix();
		 s_drawQuadIndex += 1;
    }
}

}

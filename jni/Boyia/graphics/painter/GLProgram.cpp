#include "GLProgram.h"
#include "VertexSource.h"
#include "FragmentSource.h"
#include "ShaderUtil.h"
#include "GLPainter.h"
//#include <GLES3/gl3.h>

namespace yanbo
{
struct AttributeLocation
{
	const char *attributeName;
	int location;
};

GLProgram::GLProgram()
    : m_normalPrgm(0)
    , m_videoPrgm(0)
{
}

GLProgram::~GLProgram()
{
	if (m_normalPrgm)
	{
		glDeleteProgram(m_normalPrgm);
	}

	if (m_videoPrgm)
	{
		glDeleteProgram(m_videoPrgm);
	}
}

void GLProgram::initAttribute(GLuint program)
{
	AttributeLocation attributes[] = {
        "aPosition", GLProgram::PROGRAM_ATTRIB_POSITION,
        "aColor", GLProgram::PROGRAM_ATTRIB_COLOR,
        "aTexCoord", GLProgram::PROGRAM_ATTRIB_TEX_COORD,
	};

	int size = sizeof(attributes) / sizeof(attributes[0]);

	while (size) {
	    glBindAttribLocation(program, attributes[--size].location, attributes[size].attributeName);
	}
}

void GLProgram::initShader()
{
	m_videoPrgm = ShaderUtil::createProgram(VertexSource::s_video, FragmentSource::s_video);
	initAttribute(m_videoPrgm);
	if (0 != ShaderUtil::linkProgram(m_videoPrgm))
	{
		m_videoMVPMatrix = glGetUniformLocation(m_videoPrgm, "uMVPMatrix");
	    // 是否是texture
		m_videoSTMatrix = glGetUniformLocation(m_videoPrgm, "uSTMatrix");
	    // 获取程序中纹理坐标属性引用id
		//m_aTextureCoord = glGetAttribLocation(m_program, "aTexCoord");
        // 获取采样器
		m_videoSampler2D = glGetUniformLocation(m_videoPrgm, "uSampler2D");
	}

	m_normalPrgm = ShaderUtil::createProgram(VertexSource::s_uielement, FragmentSource::s_uielement);
	initAttribute(m_normalPrgm);
	if (0 != ShaderUtil::linkProgram(m_normalPrgm))
	{
		// 获取程序中顶点位置属性引用id
		//m_aPositionHandle = glGetAttribLocation(m_program, "aPosition");
		// 获取程序中顶点颜色属性引用id
		//m_aColorHandle = glGetAttribLocation(m_program, "aColor");
		// 获取程序中总变换矩阵引用id
		m_muMVPMatrixHandle = glGetUniformLocation(m_normalPrgm, "uMVPMatrix");
	    // 是否是texture
		m_uIsImageHandle = glGetUniformLocation(m_normalPrgm, "uIsImage");
	    // 获取程序中纹理坐标属性引用id
		//m_aTextureCoord = glGetAttribLocation(m_program, "aTexCoord");
        // 获取采样器
		m_uSampler2D = glGetUniformLocation(m_normalPrgm, "uSampler2D");
    }
}

void GLProgram::use(int type)
{
	if (GLPainter::EShapeVideo == type)
	{
		glUseProgram(m_videoPrgm);
	}
	else
	{
		glUseProgram(m_normalPrgm);
	}
}

bool GLProgram::available()
{
	return m_normalPrgm != 0 && m_videoPrgm != 0;
    //return m_normalPrgm != 0;
}

GLuint GLProgram::texFlag()
{
	return m_uIsImageHandle;
}

GLuint GLProgram::matrix()
{
	return m_muMVPMatrixHandle;
}

GLuint GLProgram::sampler2D()
{
    return m_uSampler2D;
}

GLuint GLProgram::videoMatrix()
{
    return m_videoMVPMatrix;
}
GLuint GLProgram::videoSTMatrix()
{
    return m_videoSTMatrix;
}
GLuint GLProgram::videoSampler2D()
{
    return m_videoSampler2D;
}

}

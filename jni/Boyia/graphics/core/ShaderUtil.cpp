#include "ShaderUtil.h"
#include "SalLog.h"

//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>

#include <stdlib.h>

namespace yanbo
{

int ShaderUtil::s_width = 0;
int ShaderUtil::s_height = 0;
int ShaderUtil::s_realWidth = 0;
int ShaderUtil::s_realHeight = 0;

GLuint ShaderUtil::loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    KFORMATLOG("ShaderUtil::loadShader Failed %s", buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint ShaderUtil::createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
    }

    return program;
}

GLuint ShaderUtil::linkProgram(GLuint& program)
{
	glLinkProgram(program);
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
	    GLint bufLength = 0;
	    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
	    if (bufLength) {
	        char* buf = (char*) malloc(bufLength);
	        if (buf) {
	            glGetProgramInfoLog(program, bufLength, NULL, buf);
	            free(buf);
	        }
	    }
	    glDeleteProgram(program);
	    program = 0;
	}

	return program;
}

void ShaderUtil::screenToGlPoint(
		int androidX,
		int androidY,
		float* glX,
		float* glY)
{
	*glX = (1.0f*(androidX - s_width/2))/(s_width/2);
	*glY = (1.0f*(s_height/2 - androidY))/(s_height/2);
//	*glX = (1.0f*androidX/s_width);
//	*glY = (1.0f*androidY/s_height);
}

void ShaderUtil::screenToGlPixel(int androidX, int androidY
            , int* glX, int* glY)
{
    *glX = androidX;
    *glY = s_height - androidY;
}

void ShaderUtil::setScreenSize(int width, int height)
{
	s_width = width;
	s_height = height;

	KFORMATLOG("screen width=%d and height=%d", width, height);
}

float ShaderUtil::screenToGlWidth(int width)
{
    return (1.0f*width)/((1.0f*s_width)/2);
}

float ShaderUtil::screenToGlHeight(int height)
{
    return (1.0f*height)/((1.0f*s_height)/2);;
}

int ShaderUtil::screenWidth()
{
	return s_width;
}

int ShaderUtil::screenHeight()
{
	return s_height;
}

void ShaderUtil::setRealScreenSize(int width, int height)
{
	s_realWidth = width;
	s_realHeight = height;
}

int ShaderUtil::viewX(int x)
{
    return x * (1.0f*s_width)/(1.0f*s_realWidth);
}

int ShaderUtil::viewY(int y)
{
	return y * (1.0f*s_height)/(1.0f*s_realHeight);
}
}

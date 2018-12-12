#ifndef ShaderUtil_h
#define ShaderUtil_h

#include <GLES3/gl3.h>

namespace yanbo
{
class ShaderUtil
{
public:
	static GLuint loadShader(GLenum shaderType
		, const char* pSource);

    static GLuint createProgram(const char* pVertexSource
		, const char* pFragmentSource);

    static GLuint linkProgram(GLuint& program);

    static void screenToGlPoint(int androidX, int androidY
    		, float* glX, float* glY);

	static void screenToGlPixel(int androidX, int androidY
			, int* glX, int* glY);

    static void setScreenSize(int width, int height);
    static void setRealScreenSize(int width, int height);

	static float screenToGlWidth(int width);

	static float screenToGlHeight(int height);

	static int screenWidth();
	static int screenHeight();

	static int viewX(int x);
	static int viewY(int y);

private:
    static int s_width;
    static int s_height;

    static int s_realWidth;
    static int s_realHeight;
};

}

#endif

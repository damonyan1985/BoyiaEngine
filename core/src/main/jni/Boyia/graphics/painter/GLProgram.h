#ifndef GLProgram_h
#define GLProgram_h

#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>
//#include "PlatformLib.h"

namespace yanbo
{
class GLProgram
{
public:
	enum
	{
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

	GLuint m_videoPositionHandle;
	GLuint m_videoColorHandle;
	GLuint m_videoTextureHandle;

private:
	GLuint m_normalPrgm;
	GLuint m_videoPrgm;
	//GLuint m_aPositionHandle;
	//GLuint m_aColorHandle;
	GLuint m_muMVPMatrixHandle;
	GLuint m_uIsImageHandle;
	//GLuint m_aTextureCoord;
	GLuint m_uSampler2D;
	// Video Handle
	GLuint m_videoMVPMatrix;
	GLuint m_videoSTMatrix;
	GLuint m_videoSampler2D;

};
}
#endif

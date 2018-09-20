#ifndef GLPainter_h
#define GLPainter_h

#include "KVector.h"
#include "LGdi.h"
#include "KRef.h"
#include "KListMap.h"
#include "MiniTextureCache.h"
#include "GLProgram.h"
#include "MiniRenderer.h"
#include <jni.h>
#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>

using namespace util;

namespace yanbo
{
class RotateInfo
{
public:
	RotateInfo();

	float rx;
	float ry;
	float rz;
};
class PaintCommand
{
public:
	PaintCommand();

	Quad       quad;
	RotateInfo rotate;
	GLuint     texId;
	LInt       type;
};

class GLPainter : public KRef
{
public:
	enum ShapeType
	{
		EShapeNone = 0,
		EShapeLine,
		EShapeRect,
		EShapeString,
		EShapeImage,
		EShapeVideo,
	};

public:
	GLPainter();
	virtual ~GLPainter();

	static void bindVBO();
	static void unbindVBO();

public:
	void setColor(const LRgb& color);

	void setRect(const LRect& rect);
	void setLine(const LPoint& p1, const LPoint& p2);
	void setImage(MiniTexture* tex, const LRect& rect);
	void setVideo(MiniTexture* tex, const LRect& rect);

	static void init();
	void paint();
	void setScale(float scale);
	void appendToBuffer();
	float* stMatrix() const;
	//Quad   m_quad; // 矩形块

protected:
	void setTexture(MiniTexture* tex, const LRect& rect);

	static GLProgram* s_program;
	static MiniRenderer* s_renderer;

	PaintCommand m_cmd;
    float  m_scale;

    float* m_stMatrix;
    static int s_drawQuadIndex;
};
}
#endif

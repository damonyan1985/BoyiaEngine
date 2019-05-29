#ifndef GLPainter_h
#define GLPainter_h

#include "KVector.h"
#include "LGdi.h"
#include "KListMap.h"
#include "MiniTextureCache.h"
#include "BoyiaRenderer.h"
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

	LInt       top;

	Quad       quad;
	RotateInfo rotate;
	GLuint     texId;
	LInt       type;
	float*     matrix;
};

class BatchCommand
{
public:
	BatchCommand();

	LInt       type;
	GLuint     texId;
	LInt       size;
	float*     matrix;
};

class BatchCommandBuffer
{
public:
	BatchCommandBuffer();

	LBool sameMaterial(GLuint texId);
	LVoid addBatchCommand();
	LVoid addBatchCommand(const PaintCommand& cmd);
	LVoid reset();

	BatchCommand buffer[INDEX_SIZE];
	LInt size;
};

class GLPainter : public BoyiaRef
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
	void setImage(MiniTexture* tex, const LRect& rect, const LRect& clipRect);
	void setVideo(MiniTexture* tex, const LRect& rect);

	static void init();
	static void paintCommand();
	static void reset();
	void paint();
	void paintImage();
	void paintQuad();
	void paintVideo();
	void setScale(float scale);
	void appendToBuffer();
	float* stMatrix() const;

protected:
	void setTexture(MiniTexture* tex, const LRect& rect, const LRect& clipRect);

	PaintCommand m_cmd;
    float  m_scale;
    float* m_stMatrix;
    static BatchCommandBuffer s_buffer;
    static int s_drawQuadIndex;
};
}
#endif

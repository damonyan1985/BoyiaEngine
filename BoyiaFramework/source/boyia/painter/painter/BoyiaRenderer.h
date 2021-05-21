#ifndef GLQuad_h
#define GLQuad_h

#include "LGdi.h"
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>

#define VBO_SIZE 1024
#define INDEX_SIZE (VBO_SIZE * 6)

namespace yanbo {
class Vec3D {
public:
    Vec3D();

    float x;
    float y;
    float z;
};

class TexVec {
public:
    TexVec();

    float u;
    float v;
};

struct Vertex {
    Vec3D vec3D;
    LColor color;
    TexVec texCoord;
};

struct Quad {
    Vertex topLeft;
    Vertex topRight;
    Vertex bottomRight;
    Vertex bottomLeft;
};

class BoyiaRenderer {
public:
    BoyiaRenderer();
    ~BoyiaRenderer();
    LVoid createVBO();
    LVoid appendQuad(const Quad& quad);
    LVoid bind();
    LVoid unbind();
    LVoid setupIndices();

private:
    LVoid bindPosition();

    Quad m_quads[VBO_SIZE];
    LUint16 m_indices[INDEX_SIZE];
    // 顶点和索引两个缓冲区
    LUint32 m_buffersVBO[2];
    LUint32 m_buffersVAO;
    // 缓冲区矩形个数
    LInt m_quadNum;
};
} // namespace yanbo
#endif

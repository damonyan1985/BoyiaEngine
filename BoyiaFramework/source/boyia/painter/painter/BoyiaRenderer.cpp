#include "BoyiaRenderer.h"
#include "GLProgram.h"

#ifdef OPENGLES_3
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#define offsetProps(st, m) ((unsigned)((char*)&((st*)0)->m - (char*)0))

#define VERTEX_SIZE sizeof(Vertex)

namespace yanbo {
Vec3D::Vec3D()
    : x(0)
    , y(0)
    , z(0)
{
}

TexVec::TexVec()
    : u(0)
    , v(0)
{
}

BoyiaRenderer::BoyiaRenderer()
    : m_quadNum(0)
{
}

BoyiaRenderer::~BoyiaRenderer()
{
    glDisableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_POSITION);
    glDisableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_COLOR);
    glDisableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_TEX_COORD);
#ifdef OPENGLES_3     
    // 解除VAO对应的属性
    glDeleteVertexArrays(1, &m_buffersVAO);
#endif    

    glDeleteBuffers(2, m_buffersVBO);
}

// 使用VAO定义数据含义
LVoid BoyiaRenderer::bindPosition()
{      
    // 启动顶点索引
    glEnableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_POSITION);
    glEnableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_COLOR);
    glEnableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_TEX_COORD);
    

    // 绘制顶点时会从缓冲区对应偏移获取相关属性
    // vertices, 设置顶点偏移量
    glVertexAttribPointer(GLProgram::PROGRAM_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (GLvoid*)offsetProps(Vertex, vec3D));

    // colors, 设置颜色偏移量
    glVertexAttribPointer(GLProgram::PROGRAM_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, VERTEX_SIZE, (GLvoid*)offsetProps(Vertex, color));

    // tex coords, 设置纹理偏移量
    glVertexAttribPointer(GLProgram::PROGRAM_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (GLvoid*)offsetProps(Vertex, texCoord));    
}

LVoid BoyiaRenderer::createVBO()
{
#ifdef OPENGLES_3    
    // 创建VAO
    // VAO的作用是描述VBO中的数据，如果数据中既包含顶点坐标，顶点颜色，纹理坐标
    // 则需要VAO来进行分类描述
    glGenVertexArrays(1, &m_buffersVAO);
    glBindVertexArray(m_buffersVAO);
#endif    

    // 创建VBO
    glGenBuffers(2, m_buffersVBO);
    // 设置顶点Buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_quads[0]) * VBO_SIZE, m_quads, GL_DYNAMIC_DRAW);

    // 设置索引Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * INDEX_SIZE, m_indices, GL_STATIC_DRAW);

    bindPosition();

    // 解绑缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef OPENGLES_3  
    glBindVertexArray(0);
#endif    
}

LVoid BoyiaRenderer::appendQuad(const Quad& quad)
{
    if (m_quadNum < VBO_SIZE) {
        LMemcpy(m_quads + m_quadNum++, &quad, sizeof(Quad));
    }
}

LVoid BoyiaRenderer::bind()
{
#ifdef OPENGLES_3    
    // 绑定VAO
    glBindVertexArray(m_buffersVAO);
#endif    

    // 绑定顶点缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, m_buffersVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_quads[0]) * m_quadNum, m_quads, GL_DYNAMIC_DRAW);

    // 绑定索引缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffersVBO[1]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * INDEX_SIZE, m_indices, GL_STATIC_DRAW);
}

LVoid BoyiaRenderer::unbind()
{
#ifdef OPENGLES_3    
    // 解绑VAO
    glBindVertexArray(0);
#endif    

    // 解绑VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_quadNum = 0;
}

LVoid BoyiaRenderer::setupIndices()
{
    for (int i = 0; i < VBO_SIZE; i++) {
        m_indices[i * 6 + 0] = (GLushort)(i * 4 + 0);
        m_indices[i * 6 + 1] = (GLushort)(i * 4 + 1);
        m_indices[i * 6 + 2] = (GLushort)(i * 4 + 2);
        m_indices[i * 6 + 3] = (GLushort)(i * 4 + 2);
        m_indices[i * 6 + 4] = (GLushort)(i * 4 + 3);
        m_indices[i * 6 + 5] = (GLushort)(i * 4 + 0);
    }
}
} // namespace yanbo

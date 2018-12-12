#include "MiniRenderer.h"
#include "GLProgram.h"
#include <GLES3/gl3.h>

#define offsetProps(st, m) ((unsigned)((char*)&((st*)0)->m - (char*)0))

#define VERTEX_SIZE sizeof(Vertex)

//#define glGenVertexArrays glGenVertexArraysOES
//#define glBindVertexArray glBindVertexArrayOES
//#define glDeleteVertexArrays glDeleteVertexArraysOES

namespace yanbo
{
Vec3D::Vec3D()
    : x(0)
    , y(0)
    , z(0)
{
}

TexVec::TexVec()
    : u(0), v(0)
{
}

MiniRenderer::MiniRenderer()
    : m_quadNum(0)
{
}

MiniRenderer::~MiniRenderer()
{
	// 解除VAO对应的属性
    glDisableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_POSITION);
    glDisableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_COLOR);
    glDisableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_TEX_COORD);
}

void MiniRenderer::createVBO()
{
	// 创建VAO
	glGenVertexArrays(1, &m_buffersVAO);
	glBindVertexArray(m_buffersVAO);

	// 创建VBO
	glGenBuffers(2, m_buffersVBO);
    // 设置顶点Buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffersVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_quads[0]) * VBO_SIZE, m_quads, GL_DYNAMIC_DRAW);

	// 设置索引Buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffersVBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices[0]) * INDEX_SIZE, m_indices, GL_STATIC_DRAW);

	// 启动顶点索引
	glEnableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_POSITION);
	glEnableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_COLOR);
	glEnableVertexAttribArray(GLProgram::PROGRAM_ATTRIB_TEX_COORD);

	// vertices
    glVertexAttribPointer(GLProgram::PROGRAM_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (GLvoid*) offsetProps(Vertex, vec3D));

    // colors
    glVertexAttribPointer(GLProgram::PROGRAM_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, VERTEX_SIZE, (GLvoid*) offsetProps(Vertex, color));

    // tex coords
    glVertexAttribPointer(GLProgram::PROGRAM_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (GLvoid*) offsetProps(Vertex, texCoord));


    // 解绑缓冲区
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void MiniRenderer::appendQuad(const Quad& quad)
{
	if (m_quadNum < VBO_SIZE)
	{
		LMemcpy(m_quads + m_quadNum++, &quad, sizeof(Quad));
	}
}

void MiniRenderer::bind()
{
	// 绑定VAO
	glBindVertexArray(m_buffersVAO);

	// 绑定顶点缓冲区
	glBindBuffer(GL_ARRAY_BUFFER, m_buffersVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_quads[0]) * m_quadNum, m_quads, GL_DYNAMIC_DRAW);

    // 绑定索引缓冲区
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffersVBO[1]);
}

void MiniRenderer::unbind()
{
	// 解绑VAO
	glBindVertexArray(m_buffersVAO);

	// 解绑VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_quadNum = 0;
}

void MiniRenderer::setupIndices()
{
    for(int i=0; i < VBO_SIZE; i++)
    {
    	m_indices[i*6+0] = (GLushort) (i*4+0);
    	m_indices[i*6+1] = (GLushort) (i*4+1);
    	m_indices[i*6+2] = (GLushort) (i*4+2);
    	m_indices[i*6+3] = (GLushort) (i*4+2);
    	m_indices[i*6+4] = (GLushort) (i*4+3);
    	m_indices[i*6+5] = (GLushort) (i*4+0);
    }
}
}

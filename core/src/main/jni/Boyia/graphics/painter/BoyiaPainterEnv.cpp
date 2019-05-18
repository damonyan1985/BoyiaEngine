#include "BoyiaPainterEnv.h"

namespace yanbo
{
BoyiaPainterEnv* BoyiaPainterEnv::instance()
{
    static BoyiaPainterEnv sEnv;
    return &sEnv;
}

BoyiaPainterEnv::BoyiaPainterEnv()
    : m_program(NULL)
    , m_renderer(NULL)
    , m_drawQuadIndex(0)
{
}

LVoid BoyiaPainterEnv::init()
{
    if (m_program)
    {
        delete m_program;
    }

    m_program = new GLProgram();
    m_program->initShader();

    if (m_renderer)
    {
        delete m_renderer;
    }

    m_renderer = new BoyiaRenderer();
    m_renderer->setupIndices();
    m_renderer->createVBO();
}

LVoid BoyiaPainterEnv::bindVBO()
{
    m_renderer->bind();
}

LVoid BoyiaPainterEnv::unbindVBO()
{
    m_renderer->unbind();
    m_drawQuadIndex = 0;
}

LVoid BoyiaPainterEnv::appendQuad(const Quad& quad)
{
	m_renderer->appendQuad(quad);
}

bool BoyiaPainterEnv::available()
{
	return m_program && m_program->available();
}

// LVoid BoyiaPainterEnv::use(int type)
// {
// 	m_program->use(type);
// }

GLProgram* BoyiaPainterEnv::program()
{
	return m_program;
}
}
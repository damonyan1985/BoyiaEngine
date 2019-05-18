#ifndef BoyiaPainterEnv_h
#define BoyiaPainterEnv_h

#include "GLProgram.h"
#include "BoyiaRenderer.h"

namespace yanbo
{
class BoyiaPainterEnv
{
public:
    static BoyiaPainterEnv* instance();
    LVoid init();
    LVoid bindVBO();
	LVoid unbindVBO();
	LVoid appendQuad(const Quad& quad);
	bool available();
	//LVoid use(int type);
	GLProgram* program();


private:
    BoyiaPainterEnv();

    GLProgram* m_program;
    BoyiaRenderer* m_renderer;
    int m_drawQuadIndex;
};
}

#endif
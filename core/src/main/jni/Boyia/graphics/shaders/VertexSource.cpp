#include "VertexSource.h"

namespace yanbo
{

const char* VertexSource::s_uielement =
		"#version 300 es\n"
		"uniform mat4 uMVPMatrix;\n"
	    "in vec3 aPosition;\n"
		"in vec4 aColor;\n"
		"in vec2 aTexCoord;\n"
		"out vec4 vColor;\n"
		"out vec2 vTexCoord;\n"
	    "void main() {\n"
	    "  gl_Position = uMVPMatrix * vec4(aPosition, 1);\n"
		"  vColor = aColor;\n"
		"  vTexCoord = aTexCoord;\n"
	    "}\n";

const char* VertexSource::s_tank = NULL;

const char* VertexSource::s_soldier = NULL;

const char* VertexSource::s_hero = NULL;

const char* VertexSource::s_video =
		"#version 300 es\n"
        "uniform mat4 uMVPMatrix;\n"
        "uniform mat4 uSTMatrix;\n"
        "in vec3 aPosition;\n"
		"in vec4 aColor;\n"
        "in vec4 aTexCoord;\n"
		"out vec4 vColor;\n"
        "out vec2 vTexCoord;\n"
        "void main() {\n"
        "  gl_Position = uMVPMatrix * vec4(aPosition, 1);\n"
		"  vColor = aColor;\n"
        "  vTexCoord = (uSTMatrix * aTexCoord).xy;\n"
        "}\n";
}

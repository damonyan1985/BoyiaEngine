#include "FragmentSource.h"

namespace yanbo
{

const char* FragmentSource::s_uielement =
		"#version 300 es\n"
	    "precision mediump float;\n"
		"in vec2 vTexCoord;\n"
		"in vec4 vColor;\n"
		"uniform sampler2D uSampler2D;\n"
		"uniform int uIsImage;\n"
		"out vec4 FragColor;\n"
	    "void main() {\n"
		"    if (uIsImage == 1) {\n"
		"        FragColor = texture(uSampler2D, vTexCoord) * vColor; \n"
		"    } else {\n"
		"        FragColor = vColor;\n"
		"    }\n"
	    "}\n";

const char* FragmentSource::s_tank = NULL;

const char* FragmentSource::s_soldier = NULL;

const char* FragmentSource::s_hero = NULL;

const char* FragmentSource::s_video =
		"#version 300 es\n"
        "#extension GL_OES_EGL_image_external_essl3 : require\n"
        "precision mediump float;\n"
        "uniform samplerExternalOES uSampler2D;\n"
        "in vec2 vTexCoord;\n"
		"in vec4 vColor;\n"
		"out vec4 FragColor;\n"
        "void main() {\n"
        "  FragColor = texture(uSampler2D, vTexCoord);\n"
        "}\n";
}

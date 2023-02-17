#include "VertexSource.h"

namespace yanbo {

#ifdef OPENGLES_3
const char* VertexSource::s_uielement = "#version 300 es\n"
                                        "uniform mat4 uMVPMatrix;\n"
                                        "in vec3 aPosition;\n"
                                        "in vec4 aColor;\n"
                                        "in vec2 aTexCoord;\n"
                                        "out vec4 vColor;\n"
                                        "out vec2 vTexCoord;\n"
                                        "out vec4 vPosition;\n"
                                        "void main() {\n"
                                        "  vPosition = uMVPMatrix * vec4(aPosition, 1);\n"
                                        "  vColor = aColor;\n"
                                        "  vTexCoord = aTexCoord;\n"
                                        "  gl_Position = vPosition;\n"
                                        "}\n";

const char* VertexSource::s_video = "#version 300 es\n"
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

#else
const char* VertexSource::s_uielement = "uniform mat4 uMVPMatrix;\n"
                                        "attribute vec3 aPosition;\n"
                                        "attribute vec4 aColor;\n"
                                        "attribute vec2 aTexCoord;\n"
                                        "varying vec4 vColor;\n"
                                        "varying vec2 vTexCoord;\n"
                                        "varying vec4 vPosition;\n"
                                        "void main() {\n"
                                        "  vPosition = uMVPMatrix * vec4(aPosition, 1);\n"
                                        "  vColor = aColor;\n"
                                        "  vTexCoord = aTexCoord;\n"
                                        "  gl_Position = vPosition;\n"
                                        "}\n";

const char* VertexSource::s_video = "uniform mat4 uMVPMatrix;\n"
                                    "uniform mat4 uSTMatrix;\n"
                                    "attribute vec3 aPosition;\n"
                                    "attribute vec4 aColor;\n"
                                    "attribute vec4 aTexCoord;\n"
                                    "varying vec4 vColor;\n"
                                    "varying vec2 vTexCoord;\n"
                                    "void main() {\n"
                                    "  gl_Position = uMVPMatrix * vec4(aPosition, 1);\n"
                                    "  vColor = aColor;\n"
                                    "  vTexCoord = (uSTMatrix * aTexCoord).xy;\n"
                                    "}\n";
#endif
} // namespace yanbo

#include "FragmentSource.h"

namespace yanbo {
// uIsImage == 1, 表示是纹理
// uIsImage == 2, 表示是圆角矩形
// uIsImage == 0, 表示是普通矩形
const char* FragmentSource::s_uielement = "#version 300 es\n"
                                          "precision mediump float;\n"
                                          "in vec2 vTexCoord;\n"
                                          "in vec4 vColor;\n"
                                          "uniform sampler2D uSampler2D;\n"
                                          "uniform int uIsImage;\n"
                                          "uniform float uRadius;\n"
                                          "out vec4 FragColor;\n"
                                          "void main() {\n"
                                          "    if (uIsImage == 1) {\n"
                                          "        FragColor = texture(uSampler2D, vTexCoord) * vColor; \n"
                                          "    } else if (uIsImage == 2) {\n"
                                          "        float x = gl_FragCoord.x;\n"
                                          "        float y = gl_FragCoord.y;\n"
                                          "    } else {\n"
                                          "        FragColor = vColor;\n"
                                          "    }\n"
                                          "}\n";

const char* FragmentSource::s_video = "#version 300 es\n"
                                      "#extension GL_OES_EGL_image_external_essl3 : require\n"
                                      "precision mediump float;\n"
                                      "uniform samplerExternalOES uSampler2D;\n"
                                      "in vec2 vTexCoord;\n"
                                      "in vec4 vColor;\n"
                                      "out vec4 FragColor;\n"
                                      "void main() {\n"
                                      "  FragColor = texture(uSampler2D, vTexCoord);\n"
                                      "}\n";

const char* FragmentSource::s_corner = "#version 300 es\n"
                                       "precision mediump float;\n"
                                       "in vec3 vPosition;\n" // 顶点坐标
                                       "uniform vec2 uSize;\n" // 矩形大小
                                       "uniform float uRadius;\n" // 圆角半径
                                       "in vec4 vColor;\n" // 背景颜色
                                       "vec2 topLeft = vec2(1.0) + uRadius;\n"
                                       "vec2 topRight = vec2(uSize.x - uRadius, 1.0 + uRadius);\n"
                                       "vec2 bottomLeft = vec2(1.0 + uRadius, uSize.y - uRadius);\n"
                                       "vec2 bottomRight = uSize - uRadius;\n"
                                       "out vec4 FragColor;\n"
                                       "float isCorner() {\n"
                                       "  float d = 0.0;\n"
                                       "  if (vPosition.x < uRadius + 1.0 && vPosition.y < uRadius + 1.0) {\n"
                                       "    d = distance(vPosition, topLeft);\n"
                                       "  }\n"
                                       "  if (vPosition.x > uSize.x - uRadius && vPosition.y < uRadius + 1.0) {\n"
                                       "    d = distance(vPosition, topRight);\n"
                                       "  }\n"
                                       "  if (vPosition.x < uRadius + 1.0 && vPosition.y > uSize.y - uRadius) {\n"
                                       "    d = distance(vPosition, bottomLeft);\n"
                                       "  }\n"
                                       "  if (vPosition.x > uSize.x - uRadius && vPosition.y > uSize.y - uRadius) {\n"
                                       "    d = distance(vPosition, bottomRight);\n"
                                       "  }\n"
                                       "  return 1.0 - smoothstep(uRadius - 1.0, uRadius + 1.0, d);\n"
                                       "}\n"
                                       "void main() {\n"
                                       "  float corner = isCorner();\n"
                                       "  if (corner == 0.0) {\n"
                                       "    discard;\n"
                                       "  } else {\n"
                                       "    FragColor = corner * vColor;\n"
                                       "  }\n"
                                       "}\n";
} // namespace yanbo

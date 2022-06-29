#include "FragmentSource.h"

namespace yanbo {
// uIsImage == 1, 表示是纹理
// uIsImage == 2, 表示是圆角矩形
// uIsImage == 0, 表示是普通矩形
const char* FragmentSource::s_uielement = "#version 300 es\n"
                                          "precision mediump float;\n"
                                          "in vec2 vTexCoord;\n"
                                          "in vec4 vColor;\n"
                                          "in vec4 vPosition;\n"
                                          "uniform sampler2D uSampler2D;\n"
                                          "uniform int uIsImage;\n"
                                          "uniform vec4 uRadius;\n" // 四个角的半径
                                          "uniform vec4 uRect;\n" // 矩形长宽
                                          "out vec4 FragColor;\n"
                                          "float circle() {\n"
                                          "    float px = vPosition.x;\n"
                                          "    float py = vPosition.y;\n"
                                          "    float r = uRadius.x;\n" // 左上角半径
                                          "    float rx = uRect.x + r;\n" // 左上角圆心x坐标
                                          "    float ry = uRect.y + r;\n" // 左上角圆心y坐标
                                          "    float x = uRect.x;\n"
                                          "    float y = uRect.y;\n"
                                          "    if (px <= rx && py <= ry && r > 0.) {\n"
                                          "        float dist = distance(vec2(x, y), vec2(px, py));\n"
                                          "        float delta = smoothstep(r-0.001, r+0.001, dist);\n"
                                          "        return 1.0-delta;\n"
                                          "    }\n"
                                          "    return 1.0;\n"
                                          "}\n"
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
} // namespace yanbo

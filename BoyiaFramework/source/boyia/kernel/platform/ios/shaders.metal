//
//  boyia.metal
//  core
//
//  Created by yanbo on 2021/8/20.
//

#include <metal_stdlib>
#import "ShaderType.h"

using namespace metal;

typedef struct {
  // 对标opengles2中的gl_Position全局变量
  float4 gl_Position[[position]];
  // 输出opengles2中fragment shader的颜色值
  float4 vColor;
  // 输出opengles2中fragment shader中的纹理坐标
  float2 vTexCoord;
} VertexVaryings;

//// 顶点shader实现
//struct VertexShader {
//    VertexShader(thread VertexAttributes& attributes)
//    : mAttributes(attributes) {}
//
//  // 易变变量
//  VertexVaryings mVaryings;
//
//  // 属性
//  thread VertexAttributes mAttributes;
//
//  void main() {
//    mVaryings.gl_Position = mAttributes.aPosition;
//    mVaryings.vColor = mAttributes.aColor;
//    mVaryings.vTexCoord = mAttributes.aTexCoord;
//  }
//};

// 片元shader实现
//struct FragmentShader {
//    FragmentShader(thread VertexVaryings& varyings, constant Uniforms& uniforms, texture2d<half> texture)
//        : mVaryings(varyings)
//        , mUniforms(uniforms)
//        , mTexture(texture) {}
//
//    // 对标opengles2中的gl_FragColor全局变量
//    float4 gl_FragColor;
//    Uniforms mUniforms;
//    texture2d<half> mTexture;
//
//    // 接收顶点着色器中传入的易变变量
//    thread VertexVaryings mVaryings;
//
//    void main() {
//        // 颜色
//        if (mUniforms.uType == 0) {
//            gl_FragColor = mVaryings.vColor;
//        } else { // 纹理
//            constexpr sampler textureSampler(mag_filter::linear,
//                                              min_filter::linear); // sampler是采样器
//            half4 colorSample = mTexture.sample(textureSampler, mVaryings.vTexCoord); // 得到纹理对应位置的颜色
//            gl_FragColor = float4(colorSample);
//        }
//
//    }
//};

// 圆角绘制
// radius半径以及圆心传入不能和顶点一样传入-1，1之间的数值
// 必须是屏幕实际坐标点和实际长度，可通过PixelRatio进行换算
// 顶点传入到shader中后会变成屏幕坐标
float opacity(thread VertexVaryings& varyings, constant Uniforms& uniforms) {
    float r = uniforms.uRadius.radius;
    // 圆心
    float rx = uniforms.uRadius.topLeft.x;
    float ry = uniforms.uRadius.topLeft.y;
    // 坐标
    float px = varyings.gl_Position.x;
    float py = varyings.gl_Position.y;
    
    if (px <= rx && py <= ry) {
        // 抗锯齿处理
        float dist = distance(uniforms.uRadius.topLeft, float2(px, py));
        float delta = smoothstep(r-1, r+1, dist);
        return 1-delta;
    }

    return 1;
}

// 顶点着色器对外接口
vertex VertexVaryings vertexMain(uint vertexID [[vertex_id]], constant VertexAttributes* attributes [[buffer(0)]]) {
    
    VertexVaryings varyings;
    varyings.gl_Position = attributes[vertexID].aPosition;
    varyings.vColor = attributes[vertexID].aColor;
    varyings.vTexCoord = attributes[vertexID].aTexCoord;
    
    return varyings;
}

// 片元着色器对外接口
// half是两个字节的浮点型
fragment float4 fragmentMain(VertexVaryings varyings[[stage_in]],
                             constant Uniforms& uniforms[[buffer(0)]],
                             texture2d<half> colorTexture [[texture(0)]]) {
//  FragmentShader shader(varyings, uniforms, colorTexture);
//  shader.main();
//  return shader.gl_FragColor;
    float4 gl_FragColor;
    if (uniforms.uType == 0) {
        gl_FragColor = varyings.vColor;
    } else if (uniforms.uType == 4) {
        float op = opacity(varyings, uniforms);
        //float op = varyings.gl_Position.x < uniforms.uRadius.topLeft.x ? 1 : 0;
        //varyings.gl_Position.x = varyings.gl_Position.x * 0.3;
        gl_FragColor = float4(varyings.vColor.rgb, varyings.vColor.a * op); //op * varyings.vColor;
    } else { // 纹理
        constexpr sampler textureSampler(mag_filter::linear,
                                          min_filter::linear); // sampler是采样器
        half4 colorSample = colorTexture.sample(textureSampler, varyings.vTexCoord); // 得到纹理对应位置的颜色
        gl_FragColor = float4(colorSample);
    }
    
    return gl_FragColor;
}





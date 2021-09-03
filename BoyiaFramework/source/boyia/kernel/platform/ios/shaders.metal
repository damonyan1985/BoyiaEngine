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
struct FragmentShader {
    FragmentShader(thread VertexVaryings& varyings, constant Uniforms& uniforms, texture2d<half> texture)
        : mVaryings(varyings)
        , mUniforms(uniforms)
        , mTexture(texture) {}

    // 对标opengles2中的gl_FragColor全局变量
    float4 gl_FragColor;
    Uniforms mUniforms;
    texture2d<half> mTexture;

    // 接收顶点着色器中传入的易变变量
    thread VertexVaryings mVaryings;

    void main() {
        // 颜色
        if (mUniforms.uType == 0) {
            gl_FragColor = mVaryings.vColor;
        } else { // 纹理
            constexpr sampler textureSampler(mag_filter::linear,
                                              min_filter::linear); // sampler是采样器
            half4 colorSample = mTexture.sample(textureSampler, mVaryings.vTexCoord); // 得到纹理对应位置的颜色
            gl_FragColor = float4(colorSample);
        }
        
    }
};

// 顶点着色器对外接口
vertex VertexVaryings vertexMain(uint vertexID [[vertex_id]], constant VertexAttributes* attributes [[buffer(0)]]) {
    VertexVaryings varyings;
    varyings.gl_Position = attributes[vertexID].aPosition;
    varyings.vColor = attributes[vertexID].aColor;
    varyings.vTexCoord = attributes[vertexID].aTexCoord;
    return varyings;
}

// 片元着色器对外接口
fragment float4 fragmentMain(VertexVaryings varyings[[stage_in]],
                             constant Uniforms& uniforms[[buffer(0)]],
                             texture2d<half> colorTexture [[texture(0)]]) {
  FragmentShader shader(varyings, uniforms, colorTexture);
  shader.main();
  return shader.gl_FragColor;
}





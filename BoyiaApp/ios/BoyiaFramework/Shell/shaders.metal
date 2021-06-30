// Author yanbo
// BoyiaFramework

#include <metal_stdlib>


using namespace metal;

typedef struct {
  // 对标opengles2中的gl_Position全局变量
  float4 gl_Position[[position]];
  // 输出opengles2中fragment shader的颜色值
  float4 vColor;
  // 输出opengles2中fragment shader中的纹理坐标
  float2 vTexCoord;
} VertexVaryings;

// 顶点shader实现
struct VertexShader {
  VertexShader(thread VertexAttributes& attributes)
    : mAttributes(attributes) {}
   
  // 易变变量
  VertexVaryings mVaryings;

  // 属性
  thread VertexAttributes mAttributes;

  void main() {
    mVaryings.gl_Position = mAttributes.aPosition;
    mVaryings.vColor = mAttributes.aColor;
    mVaryings.vTexCoord = mAttributes.aTexCoord;
  }
};

// 片元shader实现
struct FragmentShader {
  FragmentShader(thread VertexVaryings& varyings)
    : mVaryings(varyings) {}

  // 对标opengles2中的gl_FragColor全局变量
  float4 gl_FragColor;

  // 接收顶点着色器中传入的易变变量
  thread VertexVaryings mVaryings;

  void main() {
    gl_FragColor = mVaryings.vColor;
  }
}

// 顶点着色器对外接口
vertex VertexVaryings vertexMain(VertexAttributes attributes[[stagin_in]]) {
  VertexShader shader(attributes);
  shader.main();
  return shader.mVaryings;
}

// 片元着色器对外接口
fragment float4 fragmentMain(VertexVaryings varyings[[stagin_in]]) {
  FragmentShader shader(varyings);
  shader.main();
  return shader.gl_FragColor;
}
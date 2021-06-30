// Author yanbo
// ShaderType.h
// BoyiaFramework

#ifndef ShaderType_h
#define ShaderType_h

#include <simd/simd.h>

typedef struct
{
    vector_float4 aPosition;
    vector_float3 aColor;
    vector_float2 aTexCoord;
} VertexAttributes;

#endif
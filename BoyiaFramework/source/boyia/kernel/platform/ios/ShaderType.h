//
//  ShaderType.h
//  core
//
//  Created by yanbo on 2021/8/20.
//

#ifndef ShaderType_h
#define ShaderType_h

#include <simd/simd.h>

typedef struct
{
    vector_float4 aPosition;
    vector_float4 aColor;
    vector_float2 aTexCoord;
} VertexAttributes;


typedef struct
{
    uint uType;
} Uniforms;

#endif /* ShaderType_h */


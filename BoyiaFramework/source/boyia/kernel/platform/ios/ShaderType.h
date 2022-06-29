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

typedef struct {
    vector_float2 topLeft;
    vector_float2 topRight;
    vector_float2 bottomLeft;
    vector_float2 bottomRight;
    float topLeftRadius;
    float topRightRadius;
    float bottomLeftRadius;
    float bottomRightRadius;
} Radius;

// uType, 0表示矩形，1表示图片，2表示视频，3表示nativeview, 4圆角矩形, 5圆角纹理
// uRadius, 圆角半径
typedef struct
{
    float uType;
    Radius uRadius;
} Uniforms;

#endif /* ShaderType_h */


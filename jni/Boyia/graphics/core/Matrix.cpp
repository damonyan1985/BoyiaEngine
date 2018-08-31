#include "Matrix.h"
#include "SalLog.h"
#include <math.h>
#include <stdlib.h>
namespace yanbo
{
const double PI = 3.1415926535897931;

#define I(_i, _j) ((_j)+ 4*(_i))
static void internalMultiplyMM(float* r, const float* lhs, const float* rhs)
{
    for (int i=0 ; i<4 ; ++i) {
        register const float rhs_i0 = rhs[ I(i,0) ];
        register float ri0 = lhs[ I(0,0) ] * rhs_i0;
        register float ri1 = lhs[ I(0,1) ] * rhs_i0;
        register float ri2 = lhs[ I(0,2) ] * rhs_i0;
        register float ri3 = lhs[ I(0,3) ] * rhs_i0;
        for (int j=1 ; j<4 ; ++j) {
            register const float rhs_ij = rhs[ I(i,j) ];
            ri0 += lhs[ I(j,0) ] * rhs_ij;
            ri1 += lhs[ I(j,1) ] * rhs_ij;
            ri2 += lhs[ I(j,2) ] * rhs_ij;
            ri3 += lhs[ I(j,3) ] * rhs_ij;
        }
        r[ I(i,0) ] = ri0;
        r[ I(i,1) ] = ri1;
        r[ I(i,2) ] = ri2;
        r[ I(i,3) ] = ri3;
    }
}

static void internalMx4transform(float x, float y, float z, float w, const float* pM, float* pDest) {
    pDest[0] = pM[0 + 4 * 0] * x + pM[0 + 4 * 1] * y + pM[0 + 4 * 2] * z + pM[0 + 4 * 3] * w;
    pDest[1] = pM[1 + 4 * 0] * x + pM[1 + 4 * 1] * y + pM[1 + 4 * 2] * z + pM[1 + 4 * 3] * w;
    pDest[2] = pM[2 + 4 * 0] * x + pM[2 + 4 * 1] * y + pM[2 + 4 * 2] * z + pM[2 + 4 * 3] * w;
    pDest[3] = pM[3 + 4 * 0] * x + pM[3 + 4 * 1] * y + pM[3 + 4 * 2] * z + pM[3 + 4 * 3] * w;
}

static void internalMultiplyMV(float* r, const float* lhs, const float* rhs)
{
	internalMx4transform(rhs[0], rhs[1], rhs[2], rhs[3], lhs, r);
}

KVector<float> Matrix::s_temp(32);

void Matrix::multiplyMV(KVector<float>& resultVec,
            int resultVecOffset, const KVector<float>& lhsMat, int lhsMatOffset,
            const KVector<float>& rhsVec, int rhsVecOffset)
{
	float* resultData = (float*)resultVec.getBuffer() + resultVecOffset;
	const float* lhsData = (const float*)lhsMat.getBuffer() + lhsMatOffset;
	const float* rhsData = (const float*)rhsVec.getBuffer() + rhsVecOffset;
	internalMultiplyMV(resultData, lhsData, rhsData);
}

void Matrix::multiplyMM(KVector<float>& result, int resultOffset,
		 const KVector<float>& lhs, int lhsOffset,  const KVector<float>& rhs, int rhsOffset)
{
	if(result.size() - resultOffset < 16
			|| lhs.size() - lhsOffset < 16
			|| rhs.size() - rhsOffset < 16)
	{
		return;
	}

    float* resultData = (float*)result.getBuffer() + resultOffset;
    const float* lhsData = (const float*)lhs.getBuffer() + lhsOffset;
    const float* rhsData = (const float*)rhs.getBuffer() + rhsOffset;
    internalMultiplyMM(resultData, lhsData, rhsData);
}

void Matrix::transposeM(KVector<float>& mTrans, int mTransOffset, const KVector<float>& m,
	            int mOffset)
{
    for (int i = 0; i < 4; ++i) {
        int mBase = i * 4 + mOffset;
        mTrans[i + mTransOffset] = m[mBase];
        mTrans[i + 4 + mTransOffset] = m[mBase + 1];
        mTrans[i + 8 + mTransOffset] = m[mBase + 2];
        mTrans[i + 12 + mTransOffset] = m[mBase + 3];
    }
}

bool Matrix::invertM(KVector<float>& mInv, int mInvOffset, const KVector<float>& m,
        int mOffset)
{
    // Invert a 4 x 4 matrix using Cramer's Rule

    // transpose matrix
    const float src0  = m[mOffset +  0];
    const float src4  = m[mOffset +  1];
    const float src8  = m[mOffset +  2];
    const float src12 = m[mOffset +  3];

    const float src1  = m[mOffset +  4];
    const float src5  = m[mOffset +  5];
    const float src9  = m[mOffset +  6];
    const float src13 = m[mOffset +  7];

    const float src2  = m[mOffset +  8];
    const float src6  = m[mOffset +  9];
    const float src10 = m[mOffset + 10];
    const float src14 = m[mOffset + 11];

    const float src3  = m[mOffset + 12];
    const float src7  = m[mOffset + 13];
    const float src11 = m[mOffset + 14];
    const float src15 = m[mOffset + 15];

    // calculate pairs for first 8 elements (cofactors)
    const float atmp0  = src10 * src15;
    const float atmp1  = src11 * src14;
    const float atmp2  = src9  * src15;
    const float atmp3  = src11 * src13;
    const float atmp4  = src9  * src14;
    const float atmp5  = src10 * src13;
    const float atmp6  = src8  * src15;
    const float atmp7  = src11 * src12;
    const float atmp8  = src8  * src14;
    const float atmp9  = src10 * src12;
    const float atmp10 = src8  * src13;
    const float atmp11 = src9  * src12;

    // calculate first 8 elements (cofactors)
    const float dst0  = (atmp0 * src5 + atmp3 * src6 + atmp4  * src7)
                      - (atmp1 * src5 + atmp2 * src6 + atmp5  * src7);
    const float dst1  = (atmp1 * src4 + atmp6 * src6 + atmp9  * src7)
                      - (atmp0 * src4 + atmp7 * src6 + atmp8  * src7);
    const float dst2  = (atmp2 * src4 + atmp7 * src5 + atmp10 * src7)
                      - (atmp3 * src4 + atmp6 * src5 + atmp11 * src7);
    const float dst3  = (atmp5 * src4 + atmp8 * src5 + atmp11 * src6)
                      - (atmp4 * src4 + atmp9 * src5 + atmp10 * src6);
    const float dst4  = (atmp1 * src1 + atmp2 * src2 + atmp5  * src3)
                      - (atmp0 * src1 + atmp3 * src2 + atmp4  * src3);
    const float dst5  = (atmp0 * src0 + atmp7 * src2 + atmp8  * src3)
                      - (atmp1 * src0 + atmp6 * src2 + atmp9  * src3);
    const float dst6  = (atmp3 * src0 + atmp6 * src1 + atmp11 * src3)
                      - (atmp2 * src0 + atmp7 * src1 + atmp10 * src3);
    const float dst7  = (atmp4 * src0 + atmp9 * src1 + atmp10 * src2)
                      - (atmp5 * src0 + atmp8 * src1 + atmp11 * src2);

    // calculate pairs for second 8 elements (cofactors)
    const float btmp0  = src2 * src7;
    const float btmp1  = src3 * src6;
    const float btmp2  = src1 * src7;
    const float btmp3  = src3 * src5;
    const float btmp4  = src1 * src6;
    const float btmp5  = src2 * src5;
    const float btmp6  = src0 * src7;
    const float btmp7  = src3 * src4;
    const float btmp8  = src0 * src6;
    const float btmp9  = src2 * src4;
    const float btmp10 = src0 * src5;
    const float btmp11 = src1 * src4;

    // calculate second 8 elements (cofactors)
    const float dst8  = (btmp0  * src13 + btmp3  * src14 + btmp4  * src15)
                      - (btmp1  * src13 + btmp2  * src14 + btmp5  * src15);
    const float dst9  = (btmp1  * src12 + btmp6  * src14 + btmp9  * src15)
                      - (btmp0  * src12 + btmp7  * src14 + btmp8  * src15);
    const float dst10 = (btmp2  * src12 + btmp7  * src13 + btmp10 * src15)
                      - (btmp3  * src12 + btmp6  * src13 + btmp11 * src15);
    const float dst11 = (btmp5  * src12 + btmp8  * src13 + btmp11 * src14)
                      - (btmp4  * src12 + btmp9  * src13 + btmp10 * src14);
    const float dst12 = (btmp2  * src10 + btmp5  * src11 + btmp1  * src9 )
                      - (btmp4  * src11 + btmp0  * src9  + btmp3  * src10);
    const float dst13 = (btmp8  * src11 + btmp0  * src8  + btmp7  * src10)
                      - (btmp6  * src10 + btmp9  * src11 + btmp1  * src8 );
    const float dst14 = (btmp6  * src9  + btmp11 * src11 + btmp3  * src8 )
                      - (btmp10 * src11 + btmp2  * src8  + btmp7  * src9 );
    const float dst15 = (btmp10 * src10 + btmp4  * src8  + btmp9  * src9 )
                      - (btmp8  * src9  + btmp11 * src10 + btmp5  * src8 );

    // calculate determinant
    const float det =
            src0 * dst0 + src1 * dst1 + src2 * dst2 + src3 * dst3;

    if (det == 0.0f) {
        return false;
    }

    // calculate matrix inverse
    const float invdet = 1.0f / det;
    mInv[     mInvOffset] = dst0  * invdet;
    mInv[ 1 + mInvOffset] = dst1  * invdet;
    mInv[ 2 + mInvOffset] = dst2  * invdet;
    mInv[ 3 + mInvOffset] = dst3  * invdet;

    mInv[ 4 + mInvOffset] = dst4  * invdet;
    mInv[ 5 + mInvOffset] = dst5  * invdet;
    mInv[ 6 + mInvOffset] = dst6  * invdet;
    mInv[ 7 + mInvOffset] = dst7  * invdet;

    mInv[ 8 + mInvOffset] = dst8  * invdet;
    mInv[ 9 + mInvOffset] = dst9  * invdet;
    mInv[10 + mInvOffset] = dst10 * invdet;
    mInv[11 + mInvOffset] = dst11 * invdet;

    mInv[12 + mInvOffset] = dst12 * invdet;
    mInv[13 + mInvOffset] = dst13 * invdet;
    mInv[14 + mInvOffset] = dst14 * invdet;
    mInv[15 + mInvOffset] = dst15 * invdet;

    return true;
}

void Matrix::orthoM(KVector<float>& m, int mOffset,
    float left, float right, float bottom, float top,
    float near, float far) {
    if (left == right) {
        return;
    }
    if (bottom == top) {
    	return;
    }
    if (near == far) {
    	return;
    }

    const float r_width  = 1.0f / (right - left);
    const float r_height = 1.0f / (top - bottom);
    const float r_depth  = 1.0f / (far - near);
    const float x =  2.0f * (r_width);
    const float y =  2.0f * (r_height);
    const float z = -2.0f * (r_depth);
    const float tx = -(right + left) * r_width;
    const float ty = -(top + bottom) * r_height;
    const float tz = -(far + near) * r_depth;
    m[mOffset + 0] = x;
    m[mOffset + 5] = y;
    m[mOffset +10] = z;
    m[mOffset +12] = tx;
    m[mOffset +13] = ty;
    m[mOffset +14] = tz;
    m[mOffset +15] = 1.0f;
    m[mOffset + 1] = 0.0f;
    m[mOffset + 2] = 0.0f;
    m[mOffset + 3] = 0.0f;
    m[mOffset + 4] = 0.0f;
    m[mOffset + 6] = 0.0f;
    m[mOffset + 7] = 0.0f;
    m[mOffset + 8] = 0.0f;
    m[mOffset + 9] = 0.0f;
    m[mOffset + 11] = 0.0f;
}

void Matrix::frustumM(KVector<float>& m, int offset,
        float left, float right, float bottom, float top,
        float near, float far)
{
    if (left == right) {
        return;
    }
    if (top == bottom) {
    	return;
    }
    if (near == far) {
    	return;
    }
    if (near <= 0.0f) {
    	return;
    }
    if (far <= 0.0f) {
    	return;
    }
    const float r_width  = 1.0f / (right - left);
    const float r_height = 1.0f / (top - bottom);
    const float r_depth  = 1.0f / (near - far);
    const float x = 2.0f * (near * r_width);
    const float y = 2.0f * (near * r_height);
    const float A = (right + left) * r_width;
    const float B = (top + bottom) * r_height;
    const float C = (far + near) * r_depth;
    const float D = 2.0f * (far * near * r_depth);
    m[offset + 0] = x;
    m[offset + 5] = y;
    m[offset + 8] = A;
    m[offset +  9] = B;
    m[offset + 10] = C;
    m[offset + 14] = D;
    m[offset + 11] = -1.0f;
    m[offset +  1] = 0.0f;
    m[offset +  2] = 0.0f;
    m[offset +  3] = 0.0f;
    m[offset +  4] = 0.0f;
    m[offset +  6] = 0.0f;
    m[offset +  7] = 0.0f;
    m[offset + 12] = 0.0f;
    m[offset + 13] = 0.0f;
    m[offset + 15] = 0.0f;
}

void Matrix::perspectiveM(KVector<float>& m, int offset,
      float fovy, float aspect, float zNear, float zFar)
{
    float f = 1.0f / (float) tan(fovy * (PI / 360.0));
    float rangeReciprocal = 1.0f / (zNear - zFar);

    m[offset + 0] = f / aspect;
    m[offset + 1] = 0.0f;
    m[offset + 2] = 0.0f;
    m[offset + 3] = 0.0f;

    m[offset + 4] = 0.0f;
    m[offset + 5] = f;
    m[offset + 6] = 0.0f;
    m[offset + 7] = 0.0f;

    m[offset + 8] = 0.0f;
    m[offset + 9] = 0.0f;
    m[offset + 10] = (zFar + zNear) * rangeReciprocal;
    m[offset + 11] = -1.0f;

    m[offset + 12] = 0.0f;
    m[offset + 13] = 0.0f;
    m[offset + 14] = 2.0f * zFar * zNear * rangeReciprocal;
    m[offset + 15] = 0.0f;
}

float Matrix::length(float x, float y, float z)
{
    return (float) sqrt(x * x + y * y + z * z);
}

void Matrix::setIdentityM(KVector<float>& sm, int smOffset)
{
    for (int i=0 ; i<16 ; ++i)
    {
        sm[smOffset + i] = 0;
    }

    for(int i = 0; i < 16; i += 5)
    {
        sm[smOffset + i] = 1.0f;
    }
}

void Matrix::scaleM(KVector<float>& sm, int smOffset,
		const KVector<float>& m, int mOffset,
        float x, float y, float z)
{
    for (int i=0 ; i<4 ; ++i)
    {
        int smi = smOffset + i;
        int mi = mOffset + i;
        sm[     smi] = m[     mi] * x;
        sm[ 4 + smi] = m[ 4 + mi] * y;
        sm[ 8 + smi] = m[ 8 + mi] * z;
        sm[12 + smi] = m[12 + mi];
    }
}

void Matrix::scaleM(KVector<float>& m, int mOffset,
        float x, float y, float z)
{
    for (int i=0 ; i<4 ; ++i)
    {
        int mi = mOffset + i;
        m[     mi] *= x;
        m[ 4 + mi] *= y;
        m[ 8 + mi] *= z;
    }
}

void Matrix::translateM(KVector<float>& tm, int tmOffset,
		const KVector<float>& m, int mOffset,
        float x, float y, float z)
{
    for (int i=0 ; i<12 ; ++i)
    {
        tm[tmOffset + i] = m[mOffset + i];
    }
    for (int i=0 ; i<4 ; ++i)
    {
        int tmi = tmOffset + i;
        int mi = mOffset + i;
        tm[12 + tmi] = m[mi] * x + m[4 + mi] * y + m[8 + mi] * z +
            m[12 + mi];
    }
}

void Matrix::translateM(
		KVector<float>& m, int mOffset,
        float x, float y, float z)
{
    for (int i=0 ; i<4 ; ++i)
    {
        int mi = mOffset + i;
        m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }
}

void Matrix::rotateM(KVector<float>& rm, int rmOffset,
		const KVector<float>& m, int mOffset,
        float a, float x, float y, float z)
{
    setRotateM(s_temp, 0, a, x, y, z);
    multiplyMM(rm, rmOffset, m, mOffset, s_temp, 0);
}

void Matrix::rotateM(KVector<float>& m, int mOffset,
        float a, float x, float y, float z)
{
	//KLOG("Matrix::rotateM1");
    setRotateM(s_temp, 0, a, x, y, z);
    //KLOG("Matrix::rotateM2");
    multiplyMM(s_temp, 16, m, mOffset, s_temp, 0);
    //KLOG("Matrix::rotateM3");
    float* dest = (float*)(m.getBuffer()+mOffset);
    //KLOG("Matrix::rotateM4");
    const float* src = (const float*)(s_temp.getBuffer()+16);
    //KLOG("Matrix::rotateM5");
    LMemcpy(dest, src, 16*sizeof(float));
}

void Matrix::setRotateM(KVector<float>& rm, int rmOffset,
        float a, float x, float y, float z) {
    rm[rmOffset + 3] = 0;
    rm[rmOffset + 7] = 0;
    rm[rmOffset + 11]= 0;
    rm[rmOffset + 12]= 0;
    rm[rmOffset + 13]= 0;
    rm[rmOffset + 14]= 0;
    rm[rmOffset + 15]= 1;
    a *= (float) (PI / 180.0f);
    float s = (float) sin(a);
    float c = (float) cos(a);
    if (1.0f == x && 0.0f == y && 0.0f == z) {
        rm[rmOffset + 5] = c;   rm[rmOffset + 10]= c;
        rm[rmOffset + 6] = s;   rm[rmOffset + 9] = -s;
        rm[rmOffset + 1] = 0;   rm[rmOffset + 2] = 0;
        rm[rmOffset + 4] = 0;   rm[rmOffset + 8] = 0;
        rm[rmOffset + 0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[rmOffset + 0] = c;   rm[rmOffset + 10]= c;
        rm[rmOffset + 8] = s;   rm[rmOffset + 2] = -s;
        rm[rmOffset + 1] = 0;   rm[rmOffset + 4] = 0;
        rm[rmOffset + 6] = 0;   rm[rmOffset + 9] = 0;
        rm[rmOffset + 5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[rmOffset + 0] = c;   rm[rmOffset + 5] = c;
        rm[rmOffset + 1] = s;   rm[rmOffset + 4] = -s;
        rm[rmOffset + 2] = 0;   rm[rmOffset + 6] = 0;
        rm[rmOffset + 8] = 0;   rm[rmOffset + 9] = 0;
        rm[rmOffset + 10]= 1;
    } else {
        float len = length(x, y, z);
        if (1.0f != len) {
            float recipLen = 1.0f / len;
            x *= recipLen;
            y *= recipLen;
            z *= recipLen;
        }
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        rm[rmOffset +  0] = x*x*nc +  c;
        rm[rmOffset +  4] =  xy*nc - zs;
        rm[rmOffset +  8] =  zx*nc + ys;
        rm[rmOffset +  1] =  xy*nc + zs;
        rm[rmOffset +  5] = y*y*nc +  c;
        rm[rmOffset +  9] =  yz*nc - xs;
        rm[rmOffset +  2] =  zx*nc - ys;
        rm[rmOffset +  6] =  yz*nc + xs;
        rm[rmOffset + 10] = z*z*nc +  c;
    }
}

void Matrix::setRotateEulerM(KVector<float>& rm, int rmOffset,
        float x, float y, float z) {
    x *= (float) (PI / 180.0f);
    y *= (float) (PI / 180.0f);
    z *= (float) (PI / 180.0f);
    float cx = (float) cos(x);
    float sx = (float) sin(x);
    float cy = (float) cos(y);
    float sy = (float) sin(y);
    float cz = (float) cos(z);
    float sz = (float) sin(z);
    float cxsy = cx * sy;
    float sxsy = sx * sy;

    rm[rmOffset + 0]  =   cy * cz;
    rm[rmOffset + 1]  =  -cy * sz;
    rm[rmOffset + 2]  =   sy;
    rm[rmOffset + 3]  =  0.0f;

    rm[rmOffset + 4]  =  cxsy * cz + cx * sz;
    rm[rmOffset + 5]  = -cxsy * sz + cx * cz;
    rm[rmOffset + 6]  =  -sx * cy;
    rm[rmOffset + 7]  =  0.0f;

    rm[rmOffset + 8]  = -sxsy * cz + sx * sz;
    rm[rmOffset + 9]  =  sxsy * sz + sx * cz;
    rm[rmOffset + 10] =  cx * cy;
    rm[rmOffset + 11] =  0.0f;

    rm[rmOffset + 12] =  0.0f;
    rm[rmOffset + 13] =  0.0f;
    rm[rmOffset + 14] =  0.0f;
    rm[rmOffset + 15] =  1.0f;
}

void Matrix::setLookAtM(KVector<float>& rm, int rmOffset,
         float eyeX, float eyeY, float eyeZ,
         float centerX, float centerY, float centerZ, float upX, float upY,
         float upZ) {

     // See the OpenGL GLUT documentation for gluLookAt for a description
     // of the algorithm. We implement it in a straightforward way:

     float fx = centerX - eyeX;
     float fy = centerY - eyeY;
     float fz = centerZ - eyeZ;

     // Normalize f
     float rlf = 1.0f / length(fx, fy, fz);
     fx *= rlf;
     fy *= rlf;
     fz *= rlf;

     // compute s = f x up (x means "cross product")
     float sx = fy * upZ - fz * upY;
     float sy = fz * upX - fx * upZ;
     float sz = fx * upY - fy * upX;

     // and normalize s
     float rls = 1.0f / length(sx, sy, sz);
     sx *= rls;
     sy *= rls;
     sz *= rls;

     // compute u = s x f
     float ux = sy * fz - sz * fy;
     float uy = sz * fx - sx * fz;
     float uz = sx * fy - sy * fx;

     rm[rmOffset + 0] = sx;
     rm[rmOffset + 1] = ux;
     rm[rmOffset + 2] = -fx;
     rm[rmOffset + 3] = 0.0f;

     rm[rmOffset + 4] = sy;
     rm[rmOffset + 5] = uy;
     rm[rmOffset + 6] = -fy;
     rm[rmOffset + 7] = 0.0f;

     rm[rmOffset + 8] = sz;
     rm[rmOffset + 9] = uz;
     rm[rmOffset + 10] = -fz;
     rm[rmOffset + 11] = 0.0f;

     rm[rmOffset + 12] = 0.0f;
     rm[rmOffset + 13] = 0.0f;
     rm[rmOffset + 14] = 0.0f;
     rm[rmOffset + 15] = 1.0f;

     translateM(rm, rmOffset, -eyeX, -eyeY, -eyeZ);
 }

}

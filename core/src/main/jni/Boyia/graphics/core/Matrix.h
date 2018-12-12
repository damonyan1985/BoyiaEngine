#ifndef Matrix_h
#define Matrix_h

#include "KVector.h"
namespace yanbo
{
class Matrix
{
public:
	static void multiplyMV(KVector<float>& resultVec,
            int resultVecOffset, const KVector<float>& lhsMat, int lhsMatOffset,
            const KVector<float>& rhsVec, int rhsVecOffset);

	static void multiplyMM(KVector<float>& result, int resultOffset,
			 const KVector<float>& lhs, int lhsOffset,  const KVector<float>& rhs, int rhsOffset);

	static void transposeM(KVector<float>& mTrans, int mTransOffset, const KVector<float>& m,
	            int mOffset);

	static bool invertM(KVector<float>& mInv, int mInvOffset, const KVector<float>& m,
	        int mOffset);

	static void orthoM(KVector<float>& m, int mOffset,
	    float left, float right, float bottom, float top,
	    float near, float far);

	static void frustumM(KVector<float>& m, int offset,
	        float left, float right, float bottom, float top,
	        float near, float far);

    static void perspectiveM(KVector<float>& m, int offset,
        float fovy, float aspect, float zNear, float zFar);

    static float length(float x, float y, float z);

    static void setIdentityM(KVector<float>& sm, int smOffset);

    static void scaleM(KVector<float>& sm, int smOffset,
    		const KVector<float>& m, int mOffset,
            float x, float y, float z);

    static void scaleM(KVector<float>& m, int mOffset,
            float x, float y, float z);

    static void translateM(KVector<float>& tm, int tmOffset,
    		const KVector<float>& m, int mOffset,
            float x, float y, float z);

    static void translateM(
    		KVector<float>& m, int mOffset,
            float x, float y, float z);

    static void rotateM(KVector<float>& rm, int rmOffset,
    		const KVector<float>& m, int mOffset,
            float a, float x, float y, float z);

    static void rotateM(KVector<float>& m, int mOffset,
            float a, float x, float y, float z);

    static void setRotateM(KVector<float>& rm, int rmOffset,
            float a, float x, float y, float z);

    static void setRotateEulerM(KVector<float>& rm, int rmOffset,
            float x, float y, float z);

    static void setLookAtM(KVector<float>& rm, int rmOffset,
             float eyeX, float eyeY, float eyeZ,
             float centerX, float centerY, float centerZ, float upX, float upY,
             float upZ);

private:
	static KVector<float> s_temp;
};
}
#endif

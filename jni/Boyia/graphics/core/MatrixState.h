#ifndef MatrixState_h
#define MatrixState_h

#include "Stack.h"
namespace yanbo
{

class MatrixState
{
public:
	static void init();
	static void destroy();
	static void setInitStack();
	static void pushMatrix();
	static void popMatrix();
	static void translate(float x,float y,float z);
	static void rotate(float angle,float x,float y,float z);
	static void scale(float x,float y,float z);
	static void matrix(const KVector<float>& self);
    static void setCamera
    (
		float cx,	//摄像机位置x
		float cy,   //摄像机位置y
		float cz,   //摄像机位置z
		float tx,   //摄像机目标点x
		float ty,   //摄像机目标点y
		float tz,   //摄像机目标点z
		float upx,  //摄像机UP向量X分量
		float upy,  //摄像机UP向量Y分量
		float upz   //摄像机UP向量Z分量
    );

    static void setProjectFrustum
    (
    	float left,		//near面的left
    	float right,    //near面的right
    	float bottom,   //near面的bottom
    	float top,      //near面的top
    	float near,		//near面距离
    	float far       //far面距离
    );

    static void setProjectOrtho
    (
    	float left,		//near面的left
    	float right,    //near面的right
    	float bottom,   //near面的bottom
    	float top,      //near面的top
    	float near,		//near面距离
    	float far       //far面距离
    );

    static void copyMVMatrix();
    static KVector<float>* getFinalMatrix();
    static KVector<float>* getMMatrix();
    static void setLightLocation(float x,float y,float z);

private:
    static KVector<float>* m_projMatrix;
    static KVector<float>* m_vMatrix;
    static KVector<float>* m_currMatrix;
    static KVector<float>* m_lightLocation;
    static KVector<float>* m_cameraLocation;
    static KVector<float>* m_lightPositionFB;
    static Stack<KVector<float>* >* m_stack;
    static KVector<float>* m_mVPMatrix;
    static KVector<float>* m_vMatrixForSpecFrame;
};

}
#endif

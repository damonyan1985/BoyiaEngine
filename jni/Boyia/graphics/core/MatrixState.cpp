#include "MatrixState.h"
#include "Matrix.h"
namespace yanbo
{
KVector<float>* MatrixState::m_projMatrix = NULL;
KVector<float>* MatrixState::m_vMatrix = NULL;
KVector<float>* MatrixState::m_currMatrix = NULL;
KVector<float>* MatrixState::m_lightLocation = NULL;
KVector<float>* MatrixState::m_cameraLocation = NULL;
KVector<float>* MatrixState::m_mVPMatrix = NULL;
KVector<float>* MatrixState::m_vMatrixForSpecFrame = NULL;
Stack<KVector<float>* >* MatrixState::m_stack = NULL;

void MatrixState::init()
{
	if (!m_projMatrix)
	{
		m_projMatrix = new KVector<float>(16);
		m_vMatrix = new KVector<float>(16);
		m_currMatrix = new KVector<float>(16);
		m_mVPMatrix = new KVector<float>(16);
		m_lightLocation = new KVector<float>(3);
		m_cameraLocation = new KVector<float>(3);
		m_vMatrixForSpecFrame = new KVector<float>(16);
		(*m_lightLocation)[0] = 0;
		(*m_lightLocation)[1] = 0;
		(*m_lightLocation)[2] = 0;
		m_stack = new Stack<KVector<float>* >(10);
	}
	else
	{
		(*m_lightLocation)[0] = 0;
		(*m_lightLocation)[1] = 0;
		(*m_lightLocation)[2] = 0;
	}

}

void MatrixState::destroy()
{
    delete m_projMatrix;
    delete m_vMatrix;
    delete m_currMatrix;
    delete m_mVPMatrix;
    delete m_lightLocation;
    delete m_cameraLocation;
    delete m_vMatrixForSpecFrame;
    delete m_stack;
}

void MatrixState::setInitStack()
{
	Matrix::setRotateM(*m_currMatrix, 0, 0, 1, 0, 0);
}

void MatrixState::pushMatrix()
{

	KVector<float>* matrix = new KVector<float>(16);
    int idx = 16;
    while (idx--)
    {
    	(*matrix)[idx] = (*m_currMatrix)[idx];
    }
	m_stack->push(matrix);
}

void MatrixState::popMatrix()
{
	delete m_currMatrix;
	m_currMatrix = m_stack->pop();
}

void MatrixState::translate(float x,float y,float z)//设置沿xyz轴移动
{
	Matrix::translateM(*m_currMatrix, 0, x, y, z);
}

void MatrixState::rotate(float angle,float x,float y,float z)//设置绕xyz轴移动
{
	Matrix::rotateM(*m_currMatrix, 0, angle, x, y, z);
}

void MatrixState::scale(float x,float y,float z)
{
	Matrix::scaleM(*m_currMatrix, 0, x, y, z);
}

//插入自带矩阵
void MatrixState::matrix(const KVector<float>& self)
{
	KVector<float>* result = new KVector<float>(16);
	Matrix::multiplyMM(*result, 0, *m_currMatrix,0,self,0);
	delete m_currMatrix;
	m_currMatrix = result;
}

void MatrixState::setCamera
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
)
{
	Matrix::setLookAtM
    (
    		*m_vMatrix,
    		0,
    		cx,
    		cy,
    		cz,
    		tx,
    		ty,
    		tz,
    		upx,
    		upy,
    		upz
    );

	(*m_cameraLocation)[0]=cx;
	(*m_cameraLocation)[1]=cy;
	(*m_cameraLocation)[2]=cz;
}

//设置透视投影参数
void MatrixState::setProjectFrustum
(
	float left,		//near面的left
	float right,    //near面的right
	float bottom,   //near面的bottom
	float top,      //near面的top
	float near,		//near面距离
	float far       //far面距离
)
{
	Matrix::frustumM(*m_projMatrix, 0, left, right, bottom, top, near, far);
}

//设置正交投影参数
void MatrixState::setProjectOrtho
(
	float left,		//near面的left
	float right,    //near面的right
	float bottom,   //near面的bottom
	float top,      //near面的top
	float near,		//near面距离
	float far       //far面距离
)
{
	Matrix::orthoM(*m_projMatrix, 0, left, right, bottom, top, near, far);
}

void MatrixState::copyMVMatrix()
{
	for(int i = 0;i < 16;++i)
	{
		(*m_vMatrixForSpecFrame)[i] = m_vMatrix->elementAt(i);
	}
}

KVector<float>* MatrixState::getFinalMatrix()
{
	Matrix::multiplyMM(*m_mVPMatrix, 0, *m_vMatrixForSpecFrame, 0, *m_currMatrix, 0);
    Matrix::multiplyMM(*m_mVPMatrix, 0, *m_projMatrix, 0, *m_mVPMatrix, 0);
    return m_mVPMatrix;
}

KVector<float>* MatrixState::getMMatrix()
{
    return m_currMatrix;
}

void MatrixState::setLightLocation(float x,float y,float z)
{
	(*m_lightLocation)[0] = x;
	(*m_lightLocation)[1] = y;
	(*m_lightLocation)[2] = z;
}

}

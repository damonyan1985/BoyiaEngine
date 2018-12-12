#include "LoadObjectUtil.h"
#include "MiniNormal.h"
#include "AssetManager.h"
#include "StringUtils.h"
#include "KList.h"
#include "KSet.h"
#include "KListMap.h"
#include <math.h>
#include <stdlib.h>

namespace yanbo
{

float atofUtil(const String& s)
{
    return atof((const char*)s.GetBuffer());
}

KVector<LReal>* LoadObjectUtil::newVector(LReal x, LReal y, LReal z)
{
	KVector<LReal>* vector = new KVector<LReal>(3);
	vector->clear();
	vector->addElement(x);
	vector->addElement(y);
	vector->addElement(z);
	return vector;
}

KVector<LReal>* LoadObjectUtil::getCrossProduct(LReal x1, LReal y1, LReal z1,
			LReal x2, LReal y2, LReal z2)
{
	LReal A=y1*z2-y2*z1;
	LReal B=z1*x2-z2*x1;
	LReal C=x1*y2-x2*y1;

    return newVector(A, B, C);
}

KVector<LReal>* LoadObjectUtil::vectorNormal(const KVector<LReal>& vector)
{
	//求向量的模
	LReal module=(float)sqrt(vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]);
	return newVector(vector[0]/module, vector[1]/module, vector[2]/module);
}

// 从obj文件中加载携带顶点信息的物体，并自动计算每个顶点的平均法向量
void LoadObjectUtil::loadFromFileVertexOnly(const CString& fname, float rz,float gz,float bz)
{
	// 加载后物体的引用
	// LoadedObjectVertexNormalXC lo=null;
	// 原始顶点坐标列表--直接从obj文件中加载
	KList<LReal>* alv = new KList<LReal>();
	// 顶点组装面索引列表--根据面的信息从文件中加载
	KList<LInt>* alFaceIndex=new KList<LInt>();
	// 结果顶点坐标列表--按面组织好
	KList<LReal>* alvResult=new KList<LReal>();

    KVector<LReal> alt(256);
    alt.clear();
	// 平均前各个索引对应的点的法向量集合Map
	// 此KListMap的key为点的索引， value为点所在的各个面的法向量的集合
	KListMap<int, KSet<MiniNormal*>* >* hmn = new KListMap<int, KSet<MiniNormal*>* >();

	KList<LReal>* altResult=new KList<LReal>();
    if (!AssetManager::getAssetManager()->open(fname, AASSET_MODE_UNKNOWN))
    	return;

    String buffer;
	// 扫面文件，根据行类型的不同执行不同的处理逻辑
	while (AssetManager::getAssetManager()->readLine(buffer))
	{
	    // 用空格分割行中的各个组成部分
	    KVector<String>* tempsa = util::StringUtils::split(buffer, _CS(" "));
	    if (tempsa->elementAt(0) == _CS("v"))
	    {
	    	// 此行为顶点坐标
	      	// 若为顶点坐标行则提取出此顶点的XYZ坐标添加到原始顶点坐标列表中
	        alv->push(atofUtil(tempsa->elementAt(1)));
	      	alv->push(atofUtil(tempsa->elementAt(2)));
	      	alv->push(atofUtil(tempsa->elementAt(3)));
	    }
	    else if (tempsa->elementAt(0) == _CS("vt"))
	    {
	    	// 此行为纹理坐标行
	    	alt.addElement(atofUtil(tempsa->elementAt(1))*1/2.0f);
	    	alt.addElement(atofUtil(tempsa->elementAt(2))*1/2.0f);
	    }
	    else if (tempsa->elementAt(0) == _CS("f"))
	    {
	    	// 此行为三角形面
	        /*
	         *若为三角形面行则根据 组成面的顶点的索引从原始顶点坐标列表中
	         *提取相应的顶点坐标值添加到结果顶点坐标列表中，同时根据三个
	      	 *顶点的坐标计算出此面的法向量并添加到平均前各个索引对应的点
	      	 *的法向量集合组成的Map中
	      	*/

	      	int index[3];// 三个顶点索引值的数组

	      	// 计算第0个顶点的索引，并获取此顶点的XYZ三个坐标
	      	KVector<String>* tempsaList1 = util::StringUtils::split(tempsa->elementAt(1), _CS("/"));
	      	index[0]=atoi((char*)tempsaList1->elementAt(0).GetBuffer())-1;
	      	float x0=(*alv)[3*index[0]];
	      	float y0=(*alv)[3*index[0]+1];
	      	float z0=(*alv)[3*index[0]+2];
	      	alvResult->push(x0);
	      	alvResult->push(y0);
	      	alvResult->push(z0);

	      	// 计算第1个顶点的索引，并获取此顶点的XYZ三个坐标
	      	KVector<String>* tempsaList2 = util::StringUtils::split(tempsa->elementAt(2), _CS("/"));
	      	index[1]=atoi((char*)tempsaList2->elementAt(0).GetBuffer())-1;
	      	float x1=(*alv)[3*index[1]];
	      	float y1=(*alv)[3*index[1]+1];
	      	float z1=(*alv)[3*index[1]+2];
	      	alvResult->push(x1);
	      	alvResult->push(y1);
	      	alvResult->push(z1);

	      	// 计算第2个顶点的索引，并获取此顶点的XYZ三个坐标
	      	KVector<String>* tempsaList3 = util::StringUtils::split(tempsa->elementAt(3), _CS("/"));
	      	index[2]=atoi((char*)tempsaList3->elementAt(0).GetBuffer())-1;
	      	float x2=(*alv)[3*index[2]];
	      	float y2=(*alv)[3*index[2]+1];
	      	float z2=(*alv)[3*index[2]+2];
	      	alvResult->push(x2);
	      	alvResult->push(y2);
	      	alvResult->push(z2);

	      	// 记录此面的顶点索引
	      	alFaceIndex->push(index[0]);
	      	alFaceIndex->push(index[1]);
	      	alFaceIndex->push(index[2]);

	      	// 通过三角形面两个边向量0-1，0-2求叉积得到此面的法向量
	      	// 求0号点到1号点的向量
	      	float vxa=x1-x0;
	      	float vya=y1-y0;
	      	float vza=z1-z0;
	      	// 求0号点到2号点的向量
	      	float vxb=x2-x0;
	      	float vyb=y2-y0;
	      	float vzb=z2-z0;
	      	// 通过求两个向量的叉积计算法向量
	      	KVector<LReal>* vNormal = getCrossProduct
      		(
  				vxa,vya,vza,vxb,vyb,vzb
      		);

	      	for (int i=0; i < 3; i++)
	      	{
	      		int tempInxex = index[i];
	      		//记录每个索引点的法向量到平均前各个索引对应的点的法向量集合组成的Map中
	      		//获取当前索引对应点的法向量集合
	      		KSet<MiniNormal*>* hsn = (*hmn)[tempInxex];
	      		if (hsn == NULL)
	      		{
	      			//若集合不存在则创建
	      			hsn = new KSet<MiniNormal*>();
	      		}
	      		//将此点的法向量添加到集合中
	      		//由于Normal类重写了equals方法，因此同样的法向量不会重复出现在此点
	      		//对应的法向量集合中
	      		hsn->add(new MiniNormal((*vNormal)[0], (*vNormal)[1], (*vNormal)[2]));
	      		//将集合放进HsahMap中
	      		hmn->put(tempInxex, hsn);
	      	}

      		//将纹理坐标组织到结果纹理坐标列表中
      		//第0个顶点的纹理坐标
	      	//KVector<String>* tempsaList1 = util::StringUtils::split(tempsa->elementAt(1), _CS("/"));
	      	if (alt.size() > 0)
	      	{
	      	    int indexTex = atoi((char*)tempsaList1->elementAt(1).GetBuffer())-1;
      		    altResult->push(alt[indexTex*2]);
      		    altResult->push(alt[indexTex*2+1]);
      	        //第1个顶点的纹理坐标
      		    indexTex = atoi((char*)tempsaList2->elementAt(1).GetBuffer())-1;
      		    altResult->push(alt[indexTex*2]);
      		    altResult->push(alt[indexTex*2+1]);
      	        //第2个顶点的纹理坐标
      		    indexTex = atoi((char*)tempsaList3->elementAt(1).GetBuffer())-1;
      		    altResult->push(alt[indexTex*2]);
      		    altResult->push(alt[indexTex*2+1]);
	      	}
	    }
    }
	//生成顶点数组
	int size = alvResult->count();
	//float[] vXYZ=new float[size];
	KVector<LReal>* vXYZ = new KVector<LReal>((size));
	KList<LReal>::Iterator iter = alvResult->begin();
	KList<LReal>::Iterator iterEnd = alvResult->end();
	for (; iter != iterEnd; ++iter)
	{
		vXYZ->addElement(*iter);
	}

	//生成法向量数组
	//float[] nXYZ=new float[alFaceIndex.size()*3];
	KVector<LReal>* nXYZ = new KVector<LReal>(alFaceIndex->count()*3);
	KList<LInt>::Iterator iIter = alFaceIndex->begin();
	KList<LInt>::Iterator iIterEnd = alFaceIndex->end();

    for (; iIter != iIterEnd; ++iIter)
	{
	    //根据当前点的索引从Map中取出一个法向量的集合
    	KSet<MiniNormal*>* hsn = (*hmn)[(*iIter)];
	    //求出平均法向量
    	KVector<LReal>* tn = MiniNormal::getAverage(*hsn);
	    //将计算出的平均法向量存放到法向量数组中
	    nXYZ->addElement((*tn)[0]);
	    nXYZ->addElement((*tn)[1]);
	    nXYZ->addElement((*tn)[2]);
	}

    KVector<LReal>* tST = new KVector<LReal>(altResult->count());
	KList<LReal>::Iterator fIter = altResult->begin();
	KList<LReal>::Iterator fIterEnd = altResult->end();
    for (; fIter != fIterEnd; ++fIter)
	{
    	tST->addElement(*fIter);
	}
//	    创建3D物体对象
	    //lo=new LoadedObjectVertexNormalXC(vXYZ,nXYZ,rz,gz,bz);

	//return lo;
}

}

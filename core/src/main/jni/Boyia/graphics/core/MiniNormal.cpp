#include "MiniNormal.h"
#include "LoadObjectUtil.h"

const LReal DIFF =  0.0000001f; //判断两个法向量是否相同的阈值

namespace yanbo
{
MiniNormal::MiniNormal(LReal nx, LReal ny, LReal nz)
    : m_nx(nx)
    , m_ny(ny)
    , m_nz(nz)
{
}

bool MiniNormal::equals(const MiniNormal& o) const
{
    //若两个法向量XYZ三个分量的差都小于指定的阈值则认为这两个法向量相等
	if((util::LFabs(m_nx-o.m_nx) < DIFF)
			&& (util::LFabs(m_ny-o.m_ny) < DIFF)
			&& (util::LFabs(m_ny-o.m_ny) < DIFF))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool MiniNormal::operator == (const MiniNormal& n) const
{
	if(equals(n))
		return true;

	return false;
}

//由于要用到HashSet，因此一定要重写hashCode方法
int MiniNormal::hashCode()
{
	return 1;
}

//求法向量平均值的工具方法
KVector<LReal>* MiniNormal::getAverage(const KSet<MiniNormal*>& sn)
{
	//存放法向量和的数组
	KVector<LReal> result(3);
	//把集合中所有的法向量求和
//	int size = sn.size();
//	for(int i=0; i<size; i++)
//	{
//		result[0]+=sn[i].m_nx;
//		result[1]+=sn[i].m_ny;
//		result[2]+=sn[i].m_nz;
//	}
	//将求和后的法向量规格化

	KSet<MiniNormal*>::Iterator iIter = sn.begin();
	KSet<MiniNormal*>::Iterator iIterEnd = sn.end();
    for(; iIter != iIterEnd; ++iIter)
	{
		result[0] += (*iIter)->m_nx;
		result[1] += (*iIter)->m_ny;
		result[2] += (*iIter)->m_nz;
	}

	return LoadObjectUtil::vectorNormal(result);
}

}

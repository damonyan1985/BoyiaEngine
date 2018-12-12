#ifndef MiniNormal_h
#define MiniNormal_h

#include "KVector.h"
#include "KSet.h"

namespace yanbo
{

class MiniNormal
{
public:
	MiniNormal(LReal nx, LReal ny, LReal nz);
	~MiniNormal();

	bool operator == (const MiniNormal& n) const;
	bool equals(const MiniNormal& o) const;
	int hashCode();
	static KVector<LReal>* getAverage(const KSet<MiniNormal*>& sn);

protected:
	//法向量在XYZ轴上的分量
	LReal m_nx;
	LReal m_ny;
	LReal m_nz;
};
}
#endif

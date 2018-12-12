#ifndef LoadObjectUtil_h
#define LoadObjectUtil_h

#include "KVector.h"
#include "UtilString.h"
namespace yanbo
{

class LoadObjectUtil
{
public:
	static KVector<LReal>* newVector(LReal x, LReal y, LReal z);
	static KVector<LReal>*  getCrossProduct(LReal x1, LReal y1, LReal z1,
			LReal x2, LReal y2, LReal z2);

	static KVector<LReal>* vectorNormal(const KVector<LReal>& vector);

	static void loadFromFileVertexOnly(const CString& fname, float rz,float gz,float bz);
};
}
#endif

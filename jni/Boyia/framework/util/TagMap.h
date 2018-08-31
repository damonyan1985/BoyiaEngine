/**
 * TagMap
 * @Author YanBo
 */

#ifndef TagMap_h
#define TagMap_h

#include "StringUtils.h"

namespace util
{
struct TagPair
{
    LUint hash;
    LInt  tag;
};
class TagMap
{
public:
	TagMap(LInt capacity);

	LVoid put(const String& key, LInt tag);
	LVoid put(LUint hash, LInt tag);
	LInt get(const String& key);
	LInt get(const LUint hash);
    LVoid sort();
    LVoid clear();

private:
	TagPair* m_pairs;
	LInt m_size;
	LInt m_capacity;
};
}

using util::TagMap;

#endif

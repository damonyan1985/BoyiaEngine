/**
 * TagMap
 * @Author YanBo
 */

#ifndef TagMap_h
#define TagMap_h

#include "StringUtils.h"
#include "IDCreator.h"

namespace util
{
struct TagPair
{
    LUint hash;
    LIntPtr tag;
};
class TagMap
{
public:
	TagMap(LInt capacity);
	~TagMap();

	LVoid put(const String& key, LIntPtr tag);
	LVoid put(LUint hash, LIntPtr tag);
	LIntPtr get(const String& key);
	LIntPtr get(const LUint hash);
    LVoid sort();
    LVoid clear();
    LUint genKey(const String& key);

private:
	TagPair*     m_pairs;
	LInt         m_size;
	LInt         m_capacity;
	IDCreator*   m_idCreator;
};
}

using util::TagMap;

#endif

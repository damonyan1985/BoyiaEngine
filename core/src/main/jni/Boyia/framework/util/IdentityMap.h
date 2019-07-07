/**
 * IdentityMap
 * @Author YanBo
 */

#ifndef IdentityMap_h
#define IdentityMap_h

#include "IDCreator.h"
#include "StringUtils.h"

namespace util {
struct IdentityPair;
class IdentityMap {
public:
    IdentityMap(LInt capacity);
    ~IdentityMap();

    LVoid put(const String& key, LIntPtr value);
    LVoid put(LUint key, LIntPtr value);
    LIntPtr get(const String& key);
    LIntPtr get(const LUint key);
    LVoid sort();
    LVoid clear();
    LUint genKey(const String& key);

private:
    IdentityPair* m_pairs;
    LInt m_size;
    LInt m_capacity;
    IDCreator* m_idCreator;
};
}

using util::IdentityMap;

#endif

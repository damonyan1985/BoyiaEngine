#include "TagMap.h"

namespace util
{
TagMap::TagMap(LInt capacity)
    : m_size(0)
    , m_capacity(capacity)
{
	m_pairs = new TagPair[capacity];
}

LVoid TagMap::put(LUint hash, LInt tag)
{
	if (m_size >= m_capacity)
	{
		TagPair* pairs = new TagPair[2 * m_capacity];
		LMemcpy(pairs, m_pairs, m_size * sizeof(TagPair));
		// 清除原先的m_pairs内存
		delete[] m_pairs;
		// 重置m_pairs指针
		m_pairs = pairs;
	}

	m_pairs[m_size].hash = hash;
	m_pairs[m_size++].tag = tag;
}

LVoid TagMap::put(const String& key, LInt tag)
{
	put(StringUtils::genIdentify(key), tag);
}

LInt TagMap::get(const LUint hash)
{
	LInt low = 0, high = m_size - 1, mid;

	while (low <= high) {
		mid = (low + high) / 2;
		if (hash < m_pairs[mid].hash) {
			high = mid - 1;
		} else if (hash > m_pairs[mid].hash) {
			low = mid + 1;
		} else {
			return m_pairs[mid].tag;
		}
	}

	return 0;
}

LInt TagMap::get(const String& key)
{
	return get(StringUtils::genIdentify(key));
}

LVoid TagMap::sort()
{
	for (LInt i = 1; i<m_size; ++i) {
		for (LInt j = i; j - 1 >= 0 && m_pairs[j].hash<m_pairs[j - 1].hash; --j) {
			TagPair tmp;
			tmp.hash = m_pairs[j].hash;
			tmp.tag = m_pairs[j].tag;

			m_pairs[j].hash = m_pairs[j-1].hash;
			m_pairs[j].tag = m_pairs[j-1].tag;

			m_pairs[j-1].hash = tmp.hash;
			m_pairs[j-1].tag = tmp.tag;
		}
	}
}

LVoid TagMap::clear()
{
	m_size = 0;
}
}

#include "IdentityMap.h"
#include "Stack.h"

namespace util
{
struct IdentityPair
{
    LUint key;
    LIntPtr value;
};

class IdentitySort
{
public:
	static LVoid quickSort(IdentityPair* array, LInt size)
	{
        if (size <= 0)
        {
            return;
        }

        LInt left = 0;
        LInt right = size - 1;

        Stack<LInt> stack;
        stack.push(left);
        stack.push(right);
        while (stack.size())
	    {
	        LInt right = stack.pop();
	        LInt left = stack.pop();

	        LInt index = partSort(array, left, right);
	        if (index - 1 > left)// 左子数组排序
	        {
	            stack.push(left);
	            stack.push(index - 1);
	        }

	        if (index + 1 < right)// 右子数组排序
	        {
	            stack.push(index + 1);
	            stack.push(right);
	        }
	    }
    
	}

	static LVoid insertSort(IdentityPair* pairs, LInt size)
	{
		for (LInt i = 1; i < size; ++i) {
			for (LInt j = i; j - 1 >= 0 && pairs[j].key < pairs[j - 1].key; --j) {
				IdentityPair tmp;
				tmp.key = pairs[j].key;
				tmp.value = pairs[j].value;

				pairs[j].key = pairs[j-1].key;
				pairs[j].value = pairs[j-1].value;

				pairs[j-1].key = tmp.key;
				pairs[j-1].value = tmp.value;
			}
	    }
	}

private:
	// 挖坑法
	static int partSort(IdentityPair* array, int left, int right)
	{
	    IdentityPair id;
        id.key = array[right].key;
        id.value = array[right].value;
	    while (left < right)
	    {
	        while (left < right && array[left].key <= id.key)
	        {
	            ++left;
	        }
	        
	        array[right].key = array[left].key;
	        array[right].value = array[left].value;
	        
	        while (left < right && array[right].key >= id.key)
	        {
	            --right;
	        }

	        array[left].key = array[right].key;
	        array[left].value = array[right].value;  
	    }

	    array[right].key = id.key;
	    array[right].value = id.value;
	    return right;
	}

};

IdentityMap::IdentityMap(LInt capacity)
    : m_size(0)
    , m_capacity(capacity)
    , m_idCreator(NULL)
{
	m_pairs = new IdentityPair[capacity];
	m_idCreator = new IDCreator();
}

IdentityMap::~IdentityMap()
{
	delete m_pairs;
	delete m_idCreator;
}

LVoid IdentityMap::put(LUint key, LIntPtr value)
{
	if (m_size >= m_capacity)
	{
		IdentityPair* pairs = new IdentityPair[2 * m_capacity];
		LMemcpy(pairs, m_pairs, m_size * sizeof(IdentityPair));
		// 清除原先的m_pairs内存
		delete[] m_pairs;
		// 重置m_pairs指针
		m_pairs = pairs;
	}

	m_pairs[m_size].key = key;
	m_pairs[m_size++].value = value;
}

LVoid IdentityMap::put(const String& key, LIntPtr value)
{
	put(m_idCreator->genIdentify(key), value);
}

LIntPtr IdentityMap::get(const LUint key)
{
	LInt low = 0, high = m_size - 1, mid;

	while (low <= high) {
		mid = low + (high - low) / 2;
		if (key < m_pairs[mid].key) {
			high = mid - 1;
		} else if (key > m_pairs[mid].key) {
			low = mid + 1;
		} else {
			return m_pairs[mid].value;
		}
	}

	return 0;
}

LIntPtr IdentityMap::get(const String& key)
{
	return get(m_idCreator->genIdentify(key));
}

LVoid IdentityMap::sort()
{
	IdentitySort::quickSort(m_pairs, m_size);
}

LVoid IdentityMap::clear()
{
	m_size = 0;
}

LUint IdentityMap::genKey(const String& key)
{
	return m_idCreator->genIdentify(key);
}
}

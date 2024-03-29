#include "IdentityMap.h"
#include "Stack.h"

namespace util {
struct IdentityPair {
    LUint key;
    LIntPtr value;
};

class IdentitySort {
public:
    static LVoid quickSort(IdentityPair* array, LInt size)
    {
        if (size <= 0) {
            return;
        }

        Stack<LInt> stack(20);
        stack.push(0);
        stack.push(size - 1);
        while (stack.size()) {
            LInt right = stack.pop();
            LInt left = stack.pop();

            LInt index = partSort(array, left, right);
            if (index - 1 > left) // 左子数组排序
            {
                stack.push(left);
                stack.push(index - 1);
            }

            if (index + 1 < right) // 右子数组排序
            {
                stack.push(index + 1);
                stack.push(right);
            }
        }
    }

    // 与冒泡不同，冒泡是每一轮比较将最大或是最小排到最顶端
    // 建立子数组，后面的数据与前面数据相比较，不满足排序要求时进行交换
    // 此例中。i表示后一个元素，j表示排好序中数组元素
    static LVoid insertSort(IdentityPair* pairs, LInt size)
    {
        if (size <= 1) {
            return;
        }
        for (LInt i = 1; i < size; ++i) {
            for (LInt j = i; j - 1 >= 0 && pairs[j].key < pairs[j - 1].key; --j) {
                /*
                IdentityPair tmp;
                tmp.key = pairs[j].key;
                tmp.value = pairs[j].value;

                pairs[j].key = pairs[j - 1].key;
                pairs[j].value = pairs[j - 1].value;

                pairs[j - 1].key = tmp.key;
                pairs[j - 1].value = tmp.value;*/
                swap(&pairs[j], &pairs[j - 1]);
            }
        }
    }

    static LVoid heapSort(IdentityPair* arr, LInt size)
    {
        LInt i = 0;
        makeHeap(arr, size);
        for (i = size - 1; i >= 0; i--) {
            swap(&arr[i], &arr[0]);
            adjustDown(arr, 0, i);
        }
    }

private:
    // 挖坑法
    static LInt partSort(IdentityPair* array, LInt left, LInt right)
    {
        IdentityPair id;
        id.key = array[right].key;
        id.value = array[right].value;
        while (left < right) {
            // 从左边开始向右移动下标，直到不满足排序要求时，进行占坑
            // 将占坑元素放至最右端
            while (left < right && array[left].key <= id.key) {
                ++left;
            }

            // 占坑元素移至右端
            array[right].key = array[left].key;
            array[right].value = array[left].value;

            // 从右往左
            while (left < right && array[right].key >= id.key) {
                --right;
            }

            // 占坑元素移至左端
            array[left].key = array[right].key;
            array[left].value = array[right].value;
        }

        array[right].key = id.key;
        array[right].value = id.value;
        return right;
    }

    static LVoid makeHeap(IdentityPair* arr, LInt n)
    {
        LInt i = 0;
        for (i = n / 2 - 1; i >= 0; i--) //((n-1)*2)+1 =n/2-1
        {
            adjustDown(arr, i, n);
        }
    }

    static LVoid adjustDown(IdentityPair* arr, LInt i, LInt n)
    {
        LInt j = i * 2 + 1; //子节点
        while (j < n) {
            if (j + 1 < n && arr[j].key < arr[j + 1].key) //子节点中找较小的
            {
                j++;
            }
            if (arr[i].key > arr[j].key) {
                break;
            }
            swap(&arr[i], &arr[j]);
            i = j;
            j = i * 2 + 1;
        }
    }

    static LVoid swap(IdentityPair* x, IdentityPair* y)
    {
        IdentityPair tmp;
        tmp.key = x->key;
        tmp.value = x->value;

        x->key = y->key;
        x->value = y->value;

        y->key = tmp.key;
        y->value = tmp.value;
    }
};

IdentityMap::IdentityMap(LInt capacity)
    : m_size(0)
    , m_capacity(capacity)
    , m_idCreator(kBoyiaNull)
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
    if (m_size >= m_capacity) {
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
        mid = low + (high - low) / 2; // 避免使用(high + low)/2出现超出整数范围的情况
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

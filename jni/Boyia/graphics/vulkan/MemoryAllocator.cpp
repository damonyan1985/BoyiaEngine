#ifdef MINI_VULKAN

#include "MemoryAllocator.h"
#include "VkFramework.h"

///////////////////////////////////////////////////////////////////////////////

MemoryAllocator::MemoryAllocator()
{
    mFramework = NULL;

    for (int i=0; i< NUM_MEMORY_TYPES; i++)
    {
        mDeviceMemory[i] = VK_NULL_HANDLE;
        mDeviceMemoryOffset[i] = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////

MemoryAllocator::~MemoryAllocator()
{
    for (int i=0; i< NUM_MEMORY_TYPES; i++)
    {
        if (mDeviceMemory[i] != VK_NULL_HANDLE )
        {

            vkFreeMemory(mFramework->GetDevice(), mDeviceMemory[i], nullptr);
            mDeviceMemory[i] = VK_NULL_HANDLE;
            mDeviceMemoryOffset[i] = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

VkResult MemoryAllocator::AllocateMemory(VkMemoryAllocateInfo* pInfo, VkDeviceMemory* pMem, VkDeviceSize* pOffset)
{
    // Get the memory Index being requested
    int memoryIndex = pInfo->memoryTypeIndex;

    // Check if a memory block for this index has been allocated
    if (mDeviceMemory[memoryIndex] == VK_NULL_HANDLE)
    {
        // No memory block allocated..need to allocate a big one..
        VkMemoryAllocateInfo info;
        //memset(&info, 0, sizeof(VkMemoryAllocateInfo));
        info.memoryTypeIndex    = memoryIndex;
        info.allocationSize     = ALLOCATION_SIZE;
        info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        info.pNext              = nullptr;
        VkResult  err = vkAllocateMemory(mFramework->GetDevice(), &info, nullptr, &mDeviceMemory[memoryIndex] );
        if (err != VK_SUCCESS)
        {
            // out of memory or too many memory objects created.
            return err;
        }

        // Initialize the offset
        mDeviceMemoryOffset[memoryIndex] == 0;
    }

    // Set the device memory and offset values to be returned
    *pMem       = mDeviceMemory[memoryIndex];
    *pOffset    = mDeviceMemoryOffset[memoryIndex];

    // Increment the offset
    mDeviceMemoryOffset[memoryIndex] += pInfo->allocationSize;
}

///////////////////////////////////////////////////////////////////////////////

void MemoryAllocator::FreeMemory(VkDeviceMemory* pMem, VkDeviceSize* pOffset)
{
    // Do nothing..a more sophisticated memory allocator would reuse this allocation
    return;
}

#endif

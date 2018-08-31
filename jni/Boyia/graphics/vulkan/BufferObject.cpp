//                  Copyright (c) 2016-2017 QUALCOMM Technologies Inc.
//                              All Rights Reserved.
#ifdef MINI_VULKAN

#include "VkFramework.h"

///////////////////////////////////////////////////////////////////////////////

BufferObject::~BufferObject()
{
    Destroy();
}

///////////////////////////////////////////////////////////////////////////////

void BufferObject::Destroy()
{
    if (!mSample)
    {
        return;
    }

    // destroy buffer
    vkDestroyBuffer(mSample->GetDevice(), mBuffer, nullptr);

    if (mSample->GetUsingMemoryAllocator())
    {
        MemoryAllocator* pAllocator = mSample->GetMemoryAllocator();
        pAllocator->FreeMemory(&mMemory, &mMemOffset);
    }
    else
    {
        vkFreeMemory(mSample->GetDevice(), mMemory, nullptr);
    }
    mSample = nullptr;
}

///////////////////////////////////////////////////////////////////////////////

bool BufferObject::InitBuffer(VkFramework* sample, VkDeviceSize size, VkBufferUsageFlags usageFlags,
                              const void* initialData)
{
    // If we already have buffer references, destroy them
    if (mSample)
    {
        Destroy();
    }

    VkResult err = VK_SUCCESS;

    // Create our buffer object.
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.pNext  = nullptr;
    bufferCreateInfo.size   = size;
    bufferCreateInfo.usage  = usageFlags;
    bufferCreateInfo.flags  = 0;

    err = vkCreateBuffer(sample->GetDevice(), &bufferCreateInfo, nullptr, &mBuffer);
    VK_CHECK(!err);

    // Obtain the memory requirements for this buffer.
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(sample->GetDevice(), mBuffer, &mem_reqs);
    VK_CHECK(!err);

    // If initialData is provided, it must be host visible
    VkFlags memoryFlags = (initialData)?VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT:VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // And allocate memory according to those requirements.
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext            = nullptr;
    memoryAllocateInfo.allocationSize   = 0;
    memoryAllocateInfo.memoryTypeIndex  = 0;
    memoryAllocateInfo.allocationSize   = mem_reqs.size;
    bool pass = sample->GetMemoryTypeFromProperties(mem_reqs.memoryTypeBits, memoryFlags, &memoryAllocateInfo.memoryTypeIndex);
    VK_CHECK(pass);

    if (sample->GetUsingMemoryAllocator())
    {
        MemoryAllocator* pAllocator = sample->GetMemoryAllocator();
        err = pAllocator->AllocateMemory(&memoryAllocateInfo, &mMemory, &mMemOffset);
        VK_CHECK(!err);
    }
    else
    {
        err = vkAllocateMemory(sample->GetDevice(), &memoryAllocateInfo, nullptr, &mMemory);
        mMemOffset = 0;
    }
    VK_CHECK(!err);

    if (initialData != nullptr)
    {
        // Now we need to map the memory of this new allocation so the CPU can edit it.
        void *data;
        err = vkMapMemory(sample->GetDevice(), mMemory, mMemOffset, memoryAllocateInfo.allocationSize, 0, &data);
        VK_CHECK(!err);

        memcpy(data, initialData, size);

        // Unmap the memory back from the CPU.
        vkUnmapMemory(sample->GetDevice(), mMemory);
    }

    mAllocationSize = memoryAllocateInfo.allocationSize;

    // Bind our buffer to the memory.
    err = vkBindBufferMemory(sample->GetDevice(), mBuffer, mMemory, mMemOffset);
    VK_CHECK(!err);

    mDescriptorInfo.offset  = 0;
    mDescriptorInfo.buffer  = mBuffer;
    mDescriptorInfo.range   = size;

    mSample = sample;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

VertexBufferObject::VertexBufferObject()
{

}

///////////////////////////////////////////////////////////////////////////////

VertexBufferObject::~VertexBufferObject()
{

}

///////////////////////////////////////////////////////////////////////////////

void VertexBufferObject::AddBinding(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
{
    VkVertexInputBindingDescription vibd = {};
    vibd.binding        = binding;
    vibd.stride         = stride;
    vibd.inputRate      = inputRate;
    mBindings.push_back(vibd);
}

///////////////////////////////////////////////////////////////////////////////

void VertexBufferObject::AddAttribute(uint32_t binding, uint32_t location, uint32_t offset, VkFormat format)
{
    VkVertexInputAttributeDescription viad = {};
    viad.binding        = binding;
    viad.location       = location;
    viad.offset         = offset;
    viad.format         = format;
    m_attributes.push_back(viad);
}

///////////////////////////////////////////////////////////////////////////////

VkPipelineVertexInputStateCreateInfo VertexBufferObject::CreatePipelineState()
{
    assert(mBindings.size() > 0);
    assert(m_attributes.size() > 0);
    VkPipelineVertexInputStateCreateInfo visci = {};
    visci.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    visci.pNext                             = nullptr;
    visci.vertexBindingDescriptionCount     = mBindings.size();
    visci.pVertexBindingDescriptions        = &mBindings[0];
    visci.vertexAttributeDescriptionCount   = m_attributes.size();
    visci.pVertexAttributeDescriptions      = &m_attributes[0];
    return visci;
}

///////////////////////////////////////////////////////////////////////////////
#endif

#ifdef MINI_VULKAN

#include "VkFramework.h"
#include "TextureObject.h"


///////////////////////////////////////////////////////////////////////////////

bool ImageViewObject::Destroy()
{
    if (!mFramework)
    {
        //Uninitialized
        return false;
    }

    if (!mFramework->IsInitialized())
    {
        //If the framework has already been torn down, bail out.
        return false;
    }

    vkDestroyImage(mFramework->GetDevice(), mImage, nullptr);
    vkFreeMemory(mFramework->GetDevice(), mMem, nullptr);
    vkDestroyImageView(mFramework->GetDevice(), mView, nullptr);

    mFramework = nullptr;
    return true;
}

///////////////////////////////////////////////////////////////////////////////

ImageViewObject::~ImageViewObject()
{
    Destroy();
}

///////////////////////////////////////////////////////////////////////////////

void ImageViewObject::SetLayoutImmediate(VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkResult err = VK_SUCCESS;

    // We use a shared command buffer for setup operations to change layout.
    // Reset the setup command buffer
    vkResetCommandBuffer(mFramework->GetSetupCommandBuffer(), 0);

    VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};
    commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    commandBufferInheritanceInfo.pNext = NULL;
    commandBufferInheritanceInfo.renderPass = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.subpass = 0;
    commandBufferInheritanceInfo.framebuffer = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.occlusionQueryEnable = VK_FALSE;
    commandBufferInheritanceInfo.queryFlags = 0;
    commandBufferInheritanceInfo.pipelineStatistics = 0;

    VkCommandBufferBeginInfo setupCmdsBeginInfo;
    setupCmdsBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    setupCmdsBeginInfo.pNext = NULL;
    setupCmdsBeginInfo.flags = 0;
    setupCmdsBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;

    // Begin recording to the command buffer.
    vkBeginCommandBuffer(mFramework->GetSetupCommandBuffer(), &setupCmdsBeginInfo);

    mFramework->SetImageLayout(mImage, mFramework->GetSetupCommandBuffer(), aspect, oldLayout, newLayout, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,0,0);

    //SetLayout(mFramework->GetSetupCommandBuffer(), aspect, oldLayout, newLayout );

    // We are finished recording operations.
    vkEndCommandBuffer(mFramework->GetSetupCommandBuffer());

    VkCommandBuffer buffers[1];
    buffers[0] = mFramework->GetSetupCommandBuffer();

    VkSubmitInfo submit_info;
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 0;
    submit_info.pWaitSemaphores = NULL;
    submit_info.pWaitDstStageMask = NULL;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &buffers[0];
    submit_info.signalSemaphoreCount = 0;
    submit_info.pSignalSemaphores = NULL;

    // Submit to our shared greaphics queue.
    err = vkQueueSubmit(mFramework->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    assert(!err);

    // Wait for the queue to become idle.
    err = vkQueueWaitIdle(mFramework->GetGraphicsQueue());
    assert(!err);
}

///////////////////////////////////////////////////////////////////////////////

bool ImageViewObject::CreateImageView(VkFramework* framework, uint32_t width, uint32_t height, VkFormat format, VkImageAspectFlags aspect,
                                      VkImageLayout layout, VkImageUsageFlags usage, VkFlags memoryRequirementFlags, ImageViewObject* imageViewObj)
{
    VkResult   err;
    bool   pass;

    if (!imageViewObj || !framework )
    {
        return false;
    }

    imageViewObj->mFormat = format;
    imageViewObj->mFramework = framework;
    imageViewObj->mWidth = width;
    imageViewObj->mHeight = height;
    imageViewObj->mImageLayout = layout;

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext = NULL;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = imageViewObj->mFormat;
    imageCreateInfo.extent.depth = 1.0f;
    imageCreateInfo.extent.height = imageViewObj->mHeight;
    imageCreateInfo.extent.width = imageViewObj->mWidth;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = usage;
    imageCreateInfo.flags = 0;

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = NULL;
    memoryAllocateInfo.allocationSize = 0;
    memoryAllocateInfo.memoryTypeIndex = 0;

    VkMemoryRequirements mem_reqs;

    err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, NULL, &imageViewObj->mImage);
    assert(!err);

    vkGetImageMemoryRequirements(framework->GetDevice(), imageViewObj->mImage, &mem_reqs);

    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, memoryRequirementFlags, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // allocate memory
    err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &imageViewObj->mMem);
    assert(!err);

    // bind memory
    err = vkBindImageMemory(framework->GetDevice(), imageViewObj->mImage, imageViewObj->mMem, 0);
    assert(!err);


    // Transition to the required layout
    imageViewObj->SetLayoutImmediate(aspect, VK_IMAGE_LAYOUT_UNDEFINED, layout);

    // Create the image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext = NULL;
    viewCreateInfo.image = imageViewObj->mImage;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = imageViewObj->mFormat;
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    viewCreateInfo.subresourceRange.aspectMask = aspect;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;
    viewCreateInfo.flags = 0;

    err = vkCreateImageView(framework->GetDevice(), &viewCreateInfo, NULL, &imageViewObj->mView);
    assert(!err);

    // All relevant objects created
    return true;
}

///////////////////////////////////////////////////////////////////////////////

const uint8_t* TextureObject::mpStreamBuffer;
uint32_t TextureObject::mnStreamBufferIndex;
uint32_t TextureObject::mnStreamBufferSize;

///////////////////////////////////////////////////////////////////////////////

TextureObject::TextureObject()
{
    mFramework         = NULL;
    mImage          = VK_NULL_HANDLE;
    mImageLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    mMem            = VK_NULL_HANDLE;
    mView           = VK_NULL_HANDLE;
    mFormat         = VK_FORMAT_UNDEFINED;
    mWidth          = 0;
    mHeight         = 0;
}

///////////////////////////////////////////////////////////////////////////////

TextureObject::~TextureObject()
{
    Destroy();
}

///////////////////////////////////////////////////////////////////////////////

bool TextureObject::Destroy()
{
    if (!mFramework)
    {
        //Uninitialized texture, bail out
        return false;
    }

    if (!mFramework->IsInitialized())
    {
        //If the framework has already been torn down, bail out.
        return false;
    }

    vkDestroySampler(mFramework->GetDevice(), mSampler, nullptr);

    return ImageViewObject::Destroy();;
}

///////////////////////////////////////////////////////////////////////////////

uint8_t* TextureObject::LoadTGAFromMemory( const uint8_t* dataInMemory, uint32_t* pWidth, uint32_t* pHeight, VkFormat* pFormat )
{
    struct TARGA_HEADER
    {
        uint8_t   IDLength, ColormapType, ImageType;
        uint8_t   ColormapSpecification[5];
        uint16_t  XOrigin, YOrigin;
        uint16_t  ImageWidth, ImageHeight;
        uint8_t   PixelDepth;
        uint8_t   ImageDescriptor;
    };

    TARGA_HEADER header;

    assert(pWidth);
    assert(pHeight);
    assert(pFormat);
    assert(dataInMemory);

    header = *(TARGA_HEADER*)dataInMemory;
    uint8_t nPixelSize = header.PixelDepth / 8;
    (*pWidth)  = header.ImageWidth;
    (*pHeight) = header.ImageHeight;
    (*pFormat) = nPixelSize == 3 ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;

    dataInMemory += sizeof(TARGA_HEADER);

    uint8_t* pBits = new uint8_t[ nPixelSize * header.ImageWidth * header.ImageHeight ];
    memcpy(pBits, dataInMemory, nPixelSize * header.ImageWidth * header.ImageHeight );

    // Convert the image from BGRA to RGBA
    uint8_t* p = pBits;
    for (uint32_t y = 0; y < header.ImageHeight; y++)
    {
        for (uint32_t x = 0; x < header.ImageWidth; x++)
        {
            uint8_t temp = p[2];
            p[2] = p[0];
            p[0] = temp;
            p += nPixelSize;
        }
    }

    return pBits;
}

///////////////////////////////////////////////////////////////////////////////

uint32_t TextureObject::StreamRead(void* pData, unsigned int nSize)
{
    if (mnStreamBufferIndex + nSize > mnStreamBufferSize)
    {
        return 0;
    }

    memcpy(pData, &mpStreamBuffer[mnStreamBufferIndex], nSize);
    mnStreamBufferIndex += nSize;

    return nSize;
}

///////////////////////////////////////////////////////////////////////////////

uint32_t TextureObject::StreamSkip(unsigned int nSize)
{
    if (mnStreamBufferIndex + nSize > mnStreamBufferSize)
    {
        return 0;
    }

    mnStreamBufferIndex += nSize;

    return nSize;
}

///////////////////////////////////////////////////////////////////////////////

 void L_SwapEndian16(uint16_t* pData16, uint32_t nCount)
{
    for (uint32_t i = 0; i<nCount; ++i)
    {
        uint16_t x = *pData16;
        *pData16++ = (x << 8) | (x >> 8);
    }
}

///////////////////////////////////////////////////////////////////////////////

void L_SwapEndian32(uint32_t* pData32, uint32_t nCount)
{
    for (uint32_t i = 0; i<nCount; ++i)
    {
        uint32_t x = *pData32;
        *pData32++ = (x << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | (x >> 24);
    }
}

///////////////////////////////////////////////////////////////////////////////

TKTXErrorCode TextureObject::ParseHeader(TKTXHeader* pHeader, TKTXTextureInfo* pTextureInfo)
{
    uint8_t    identifierReference[12] = KTX_IDENTIFIER_REF;
    uint32_t    nMaxMipDimension;

    if (StreamRead(pHeader, KTX_HEADER_SIZE) != KTX_HEADER_SIZE)
    {
        return KTX_HEADER_ERROR;
    }

    // Compare identifier, is this a KTX file?
    if (memcmp(pHeader->identifier, identifierReference, 12) != 0)
    {
        return KTX_HEADER_ERROR;
    }

    if (pHeader->endianness == KTX_ENDIAN_REF_REV)
    {
        // Convert endianness of header fields if necessary
        L_SwapEndian32(&pHeader->glType, 12);

        if (pHeader->glTypeSize != 1 &&
            pHeader->glTypeSize != 2 &&
            pHeader->glTypeSize != 4)
        {
            // Only 8, 16, and 32-bit types supported so far
            return KTX_INVALID_VALUE;
        }
    }
    else if (pHeader->endianness != KTX_ENDIAN_REF)
    {
        return KTX_INVALID_VALUE;
    }

    // Check glType and glFormat
    pTextureInfo->bCompressed = 0;
    if (pHeader->glType == 0 || pHeader->glFormat == 0)
    {
        if (pHeader->glType + pHeader->glFormat != 0)
        {
            // either both or none of glType, glFormat must be zero
            return KTX_INVALID_VALUE;
        }
        pTextureInfo->bCompressed = 1;
    }

    // Check texture dimensions. KTX files can store 8 types of textures:
    // 1D, 2D, 3D, cube, and array variants of these. There is currently
    // no GL extension that would accept 3D array or cube array textures.
    if ((pHeader->pixelWidth == 0) ||
        (pHeader->pixelDepth > 0 && pHeader->pixelHeight == 0))
    {
        // texture must have width
        // texture must have height if it has depth
        return KTX_INVALID_VALUE;
    }

    pTextureInfo->nTextureDimensions = 1;
    pTextureInfo->glTarget = GL_TEXTURE_1D;
    if (pHeader->pixelHeight > 0)
    {
        pTextureInfo->nTextureDimensions = 2;
        pTextureInfo->glTarget = GL_TEXTURE_2D;
    }
    if (pHeader->pixelDepth > 0)
    {
        pTextureInfo->nTextureDimensions = 3;
        pTextureInfo->glTarget = GL_TEXTURE_3D;
    }

    if (pHeader->numberOfFaces == 6)
    {
        if (pTextureInfo->nTextureDimensions != 2)
        {
            // cube map needs 2D faces
            return KTX_INVALID_VALUE;
        }

        pTextureInfo->glTarget = GL_TEXTURE_CUBE_MAP;
    }
    else if (pHeader->numberOfFaces != 1)
    {
        // numberOfFaces must be either 1 or 6
        return KTX_INVALID_VALUE;
    }

    // load as 2D texture if 1D textures are not supported
    if (pTextureInfo->nTextureDimensions == 1)
    {
        pTextureInfo->nTextureDimensions = 2;
        pTextureInfo->glTarget = GL_TEXTURE_2D;
        pHeader->pixelHeight = 1;
    }

    if (pHeader->numberOfArrayElements > 0)
    {
        return KTX_UNSUPPORTED_TEXTURE_TYPE;
    }

    // reject 3D texture if unsupported
    if (pTextureInfo->nTextureDimensions == 3)
    {
        return KTX_UNSUPPORTED_TEXTURE_TYPE;
    }

    // Check number of mipmap levels
    if (pHeader->numberOfMipmapLevels == 0)
    {
        pHeader->numberOfMipmapLevels = 1;
    }
    nMaxMipDimension = MAX(MAX(pHeader->pixelWidth, pHeader->pixelHeight), pHeader->pixelDepth);
    if (nMaxMipDimension < ((uint32_t)1 << (pHeader->numberOfMipmapLevels - 1)))
    {
        // Can't have more mip levels than 1 + log2(max(width, height, depth))
        return KTX_INVALID_VALUE;
    }

    // skip key/value metadata
    if (StreamSkip(pHeader->bytesOfKeyValueData) != pHeader->bytesOfKeyValueData)
    {
        return KTX_UNEXPECTED_END_OF_STREAM;
    }

    return KTX_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

bool ConvertFormat(uint32_t internalFormat, VkFormat &textureFormat)
{
    switch (internalFormat)
    {
        case GL_RGBA:
            textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
            break;

        default:
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////

TKTXErrorCode TextureObject::LoadKTXFromMemory(VkFramework* framework, const uint8_t* dataInMemory, uint32_t filesize, TextureObject* pTextureObject)
{
    mpStreamBuffer = dataInMemory;
    mnStreamBufferSize = filesize;
    mnStreamBufferIndex=0;

    TKTXHeader      tHeader;
    TKTXTextureInfo textureInfo;

    TKTXErrorCode nErrorCode = ParseHeader(&tHeader, &textureInfo);
    if (nErrorCode != KTX_SUCCESS)
    {
        return nErrorCode;
    }

    pTextureObject->mWidth  = tHeader.pixelWidth;
    pTextureObject->mHeight = tHeader.pixelHeight;

    if (!ConvertFormat(tHeader.glBaseInternalFormat, pTextureObject->mFormat))
    {
        return KTX_UNSUPPORTED_TEXTURE_TYPE;
    }
    pTextureObject->mNumMipLevels = tHeader.numberOfMipmapLevels;
    pTextureObject->mNumFaces = tHeader.numberOfFaces;
    if (pTextureObject->mNumFaces!= 1 && pTextureObject->mNumFaces != 6)
    {
        return KTX_UNSUPPORTED_TEXTURE_TYPE;
    }

    // Initialize the CreateInfo structure
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext               = NULL;
    imageCreateInfo.imageType           = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format              = pTextureObject->mFormat;
    imageCreateInfo.extent.depth        = 1;
    imageCreateInfo.extent.width        = pTextureObject->mWidth;
    imageCreateInfo.extent.height       = pTextureObject->mHeight;
    imageCreateInfo.mipLevels           = pTextureObject->mNumMipLevels;
    imageCreateInfo.arrayLayers         = pTextureObject->mNumFaces;
    imageCreateInfo.samples             = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling              = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.usage               = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.flags               = 0;
    imageCreateInfo.sharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout       = VK_IMAGE_LAYOUT_PREINITIALIZED;

    // Initialize the memory allocation structure
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext            = NULL;
    memoryAllocateInfo.allocationSize   = 0;
    memoryAllocateInfo.memoryTypeIndex  = 0;

    // Reset the setup command buffer
    vkResetCommandBuffer(framework->GetSetupCommandBuffer(), 0);

    VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};
    commandBufferInheritanceInfo.sType                  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    commandBufferInheritanceInfo.pNext                  = NULL;
    commandBufferInheritanceInfo.renderPass             = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.subpass                = 0;
    commandBufferInheritanceInfo.framebuffer            = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.occlusionQueryEnable   = VK_FALSE;
    commandBufferInheritanceInfo.queryFlags             = 0;
    commandBufferInheritanceInfo.pipelineStatistics     = 0;

    VkCommandBufferBeginInfo setupCmdsBeginInfo;
    setupCmdsBeginInfo.sType                            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    setupCmdsBeginInfo.pNext                            = NULL;
    setupCmdsBeginInfo.flags                            = 0;
    setupCmdsBeginInfo.pInheritanceInfo                 = &commandBufferInheritanceInfo;

    // Begin recording to the command buffer.
    vkBeginCommandBuffer(framework->GetSetupCommandBuffer(), &setupCmdsBeginInfo);

    // Per miplevel values
    VkImage mipImages[pTextureObject->mNumMipLevels];
    VkDeviceMemory  mipMemory[pTextureObject->mNumMipLevels];

    VkMemoryRequirements mem_reqs;
    VkResult   err;
    bool   pass;

    // For each mip level
    for (uint32_t nMipLevel = 0; nMipLevel < pTextureObject->mNumMipLevels; nMipLevel++)
    {
        uint32_t nFaceLodSize;
        uint32_t nFaceLodSizeRounded;

        uint32_t nWidth  = MAX(1, tHeader.pixelWidth  >> nMipLevel);
        uint32_t nHeight = MAX(1, tHeader.pixelHeight >> nMipLevel);
        uint32_t nDepth  = MAX(1, tHeader.pixelDepth  >> nMipLevel);

        if (StreamRead(&nFaceLodSize, sizeof(uint32_t)) != sizeof(uint32_t))
        {
            return KTX_UNEXPECTED_END_OF_STREAM;
        }
        if (tHeader.endianness == KTX_ENDIAN_REF_REV)
        {
            L_SwapEndian32(&nFaceLodSize, 1);
        }
        nFaceLodSizeRounded = (nFaceLodSize + 3) & ~(uint32_t)3;

        uint8_t* pData = new uint8_t[nFaceLodSizeRounded * tHeader.numberOfFaces];

        for (uint32_t nFace = 0; nFace< tHeader.numberOfFaces; nFace++)
        {
            // LogInfo(0, "Reading face %d", nFace);
            if (StreamRead(pData + (nFace * nFaceLodSizeRounded), nFaceLodSizeRounded) != nFaceLodSizeRounded) {
                delete[] pData;
                return KTX_UNEXPECTED_END_OF_STREAM;
            }

            // Perform endianness conversion on texture m_pData
            if (tHeader.endianness == KTX_ENDIAN_REF_REV && tHeader.glTypeSize == 2) {
                L_SwapEndian16((uint16_t *) pData, nFaceLodSize / 2);
            }
            else if (tHeader.endianness == KTX_ENDIAN_REF_REV && tHeader.glTypeSize == 4) {
                L_SwapEndian32((uint32_t *) pData, nFaceLodSize / 4);
            }

            if (textureInfo.nTextureDimensions != 2) {
                delete[] pData;
                return KTX_UNSUPPORTED_TEXTURE_TYPE;
            }
        }

        uint32_t nMipBytes = nFaceLodSizeRounded * tHeader.numberOfFaces;

        // Set the image size for this mip level
        imageCreateInfo.extent.width  = nWidth;
        imageCreateInfo.extent.height = nHeight;
        imageCreateInfo.extent.depth  = 1;
        imageCreateInfo.mipLevels = 1;

        // Create the mip level image
        err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, NULL, &mipImages[nMipLevel]);
        assert(!err);

        vkGetImageMemoryRequirements(framework->GetDevice(), mipImages[nMipLevel], &mem_reqs);

        memoryAllocateInfo.allocationSize   = mem_reqs.size;
        pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
        assert(pass);

        // allocate memory
        err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &mipMemory[nMipLevel]);
        assert(!err);

        // bind memory
        err = vkBindImageMemory(framework->GetDevice(), mipImages[nMipLevel], mipMemory[nMipLevel], 0);
        assert(!err);

        // copy image data to the mip memory
        VkImageSubresource subRes = {};
        subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VkSubresourceLayout subResLayout;
        vkGetImageSubresourceLayout(framework->GetDevice(), mipImages[nMipLevel], &subRes, &subResLayout);
        void *data;
        err = vkMapMemory(framework->GetDevice(), mipMemory[nMipLevel], 0, mem_reqs.size, 0, &data);
        assert(!err);
        memcpy(data, pData, mem_reqs.size);
        vkUnmapMemory(framework->GetDevice(), mipMemory[nMipLevel]);

        // Change the mip image layout to transfer src
        framework->SetImageLayout(mipImages[nMipLevel],
                               framework->GetSetupCommandBuffer(),
                               VK_IMAGE_ASPECT_COLOR_BIT,
                               VK_IMAGE_LAYOUT_PREINITIALIZED,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, nMipLevel, pTextureObject->mNumMipLevels);

        delete[] pData;
    }

    // Setup texture as blit target
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.mipLevels = pTextureObject->mNumMipLevels;
    imageCreateInfo.extent = {pTextureObject->mWidth, pTextureObject->mHeight, 1 };

    // Create the image
    err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, nullptr, &pTextureObject->mImage);
    assert(!err);

    // Get the memory requirements
    vkGetImageMemoryRequirements(framework->GetDevice(), pTextureObject->mImage, &mem_reqs);

    // Get the memory type
    memoryAllocateInfo.allocationSize   = mem_reqs.size;
    pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // allocate memory
    err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &pTextureObject->mMem);
    assert(!err);

    // bind memory
    err = vkBindImageMemory(framework->GetDevice(), pTextureObject->mImage, pTextureObject->mMem, 0);
    assert(!err);

    // Change image layout to Transfer_DST so it can be filled.
    framework->SetImageLayout(pTextureObject->mImage,
                           framework->GetSetupCommandBuffer(),
                           VK_IMAGE_ASPECT_COLOR_BIT,
                           VK_IMAGE_LAYOUT_PREINITIALIZED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, pTextureObject->mNumMipLevels);

    // Copy mip levels
    for (uint32_t nMipLevel = 0; nMipLevel < pTextureObject->mNumMipLevels; nMipLevel++)
    {
        uint32_t nWidth  = MAX(1, tHeader.pixelWidth  >> nMipLevel);
        uint32_t nHeight = MAX(1, tHeader.pixelHeight >> nMipLevel);


        // Create a region for the image blit
        VkImageCopy region = {};

        region.srcSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.baseArrayLayer    = 0;
        region.srcSubresource.mipLevel          = 0;
        region.srcSubresource.layerCount        = 1;
        region.srcOffset                        = { 0, 0, 0 };
        region.dstSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstSubresource.baseArrayLayer    = 0;
        region.dstSubresource.mipLevel          = nMipLevel;
        region.dstSubresource.layerCount        = tHeader.numberOfFaces;
        region.dstOffset                        = { 0, 0, 0 };
        region.extent.width                     = nWidth;
        region.extent.height                    = nHeight;
        region.extent.depth                     = 1;

        // Put image copy for this mip level into command buffer
        vkCmdCopyImage(framework->GetSetupCommandBuffer(),
                       mipImages[nMipLevel],
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       pTextureObject->mImage,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    framework->SetImageLayout(pTextureObject->mImage,
                           framework->GetSetupCommandBuffer(),
                           VK_IMAGE_ASPECT_COLOR_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, pTextureObject->mNumMipLevels);

    // We are finished recording operations.
    vkEndCommandBuffer(framework->GetSetupCommandBuffer());

    // Prepare to submit the command buffer
    VkCommandBuffer buffers[1];
    buffers[0] = framework->GetSetupCommandBuffer();

    VkSubmitInfo submit_info;
    submit_info.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext                   = NULL;
    submit_info.waitSemaphoreCount      = 0;
    submit_info.pWaitSemaphores         = NULL;
    submit_info.pWaitDstStageMask       = NULL;
    submit_info.commandBufferCount      = 1;
    submit_info.pCommandBuffers         = &buffers[0];
    submit_info.signalSemaphoreCount    = 0;
    submit_info.pSignalSemaphores       = NULL;

    // Submit to our shared graphics queue.
    err = vkQueueSubmit(framework->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    assert(!err);

    // Wait for the queue to become idle.
    err = vkQueueWaitIdle(framework->GetGraphicsQueue());
    assert(!err);

    // Cleanup the mip structures
    for (uint32_t nMipLevel = 0; nMipLevel <pTextureObject->mNumMipLevels; nMipLevel++)
    {
        vkDestroyImage(framework->GetDevice(), mipImages[nMipLevel], nullptr);
        vkFreeMemory(  framework->GetDevice(), mipMemory[nMipLevel], nullptr);
    }

    // Now create a sampler for this image, with required details
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType                     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext                     = nullptr;
    samplerCreateInfo.magFilter                 = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter                 = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode                = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias                = 0.0f;
    samplerCreateInfo.anisotropyEnable          = VK_FALSE;
    samplerCreateInfo.maxAnisotropy             = 0;
    samplerCreateInfo.compareOp                 = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod                    = 0.0f;
    samplerCreateInfo.maxLod                    = 0.0f;
    samplerCreateInfo.borderColor               = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates   = VK_FALSE;

    err = vkCreateSampler(framework->GetDevice(), &samplerCreateInfo, NULL, &pTextureObject->mSampler);
    assert(!err);

    // Create the image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext                            = NULL;
    viewCreateInfo.image                            = VK_NULL_HANDLE;
    viewCreateInfo.viewType                         = tHeader.numberOfFaces==1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_CUBE;
    viewCreateInfo.format                           = pTextureObject->mFormat;
    viewCreateInfo.components.r                     = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g                     = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b                     = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a                     = VK_COMPONENT_SWIZZLE_A;
    viewCreateInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel    = 0;
    viewCreateInfo.subresourceRange.levelCount      = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer  = 0;
    viewCreateInfo.subresourceRange.layerCount      = 1;
    viewCreateInfo.flags                            = 0;
    viewCreateInfo.image                            = pTextureObject->mImage;

    err = vkCreateImageView(framework->GetDevice(), &viewCreateInfo, NULL, &pTextureObject->mView);
    assert(!err);

    // All relevant objects created
    return KTX_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

bool TextureObject::FromKTXFile(VkFramework* framework, const char* filename, TextureObject* pTextureObject)
{
    if (!pTextureObject || !framework || !filename)
    {
        return false;
    }

    // Get the image file from the Android asset manager
    AAsset *file = AAssetManager_open(framework->GetAssetManager(), filename, AASSET_MODE_BUFFER);
    assert(file);

    const uint8_t *file_buffer = (const uint8_t *) AAsset_getBuffer(file);
    assert(file_buffer);

    const uint32_t file_size = AAsset_getLength(file);

    TKTXErrorCode error = LoadKTXFromMemory(framework, file_buffer, file_size, pTextureObject);
    AAsset_close(file);

    return error == KTX_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

bool TextureObject::FromTGAFile(VkFramework* framework, const char* filename, TextureObject* textureObject)
{
    if (!textureObject || !framework || !filename)
    {
        return false;
    }

    // Get the image file from the Android asset manager
    AAsset *file = AAssetManager_open(framework->GetAssetManager(), filename, AASSET_MODE_BUFFER);
    assert(file);

    const uint8_t *file_buffer = (const uint8_t *) AAsset_getBuffer(file);
    assert(file_buffer);

    uint32_t *imageData = (uint32_t *) LoadTGAFromMemory(file_buffer, &textureObject->mWidth,&textureObject->mHeight, &textureObject->mFormat);
    AAsset_close(file);

    bool ret = FromTGAImageData(framework, textureObject, imageData);
    delete imageData;

    return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TextureObject::FromTGAImageData(VkFramework *framework, TextureObject *textureObject, uint32_t *imageData)
{
    // Only one format is supported here.
    assert(textureObject->mFormat == VK_FORMAT_R8G8B8A8_UNORM);
    VkResult   err;
    bool   pass;

    textureObject->mFramework = framework;

    // Initialize the CreateInfo structure
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext               = NULL;
    imageCreateInfo.imageType           = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format              = textureObject->mFormat;
    imageCreateInfo.extent.depth        = 1;
    imageCreateInfo.extent.width        = textureObject->mWidth;
    imageCreateInfo.extent.height       = textureObject->mHeight;
    imageCreateInfo.mipLevels           = 1;
    imageCreateInfo.arrayLayers         = 1;
    imageCreateInfo.samples             = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling              = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.usage               = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.flags               = 0;
    imageCreateInfo.sharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout       = VK_IMAGE_LAYOUT_PREINITIALIZED;

    VkMemoryRequirements mem_reqs;

    // Initialize the memory allocation structure
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext            = NULL;
    memoryAllocateInfo.allocationSize   = 0;
    memoryAllocateInfo.memoryTypeIndex  = 0;

    // Reset the setup command buffer
    vkResetCommandBuffer(framework->GetSetupCommandBuffer(), 0);

    VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};
    commandBufferInheritanceInfo.sType                  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    commandBufferInheritanceInfo.pNext                  = NULL;
    commandBufferInheritanceInfo.renderPass             = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.subpass                = 0;
    commandBufferInheritanceInfo.framebuffer            = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.occlusionQueryEnable   = VK_FALSE;
    commandBufferInheritanceInfo.queryFlags             = 0;
    commandBufferInheritanceInfo.pipelineStatistics     = 0;

    VkCommandBufferBeginInfo setupCmdsBeginInfo;
    setupCmdsBeginInfo.sType                            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    setupCmdsBeginInfo.pNext                            = NULL;
    setupCmdsBeginInfo.flags                            = 0;
    setupCmdsBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;

    // Begin recording to the command buffer.
    vkBeginCommandBuffer(framework->GetSetupCommandBuffer(), &setupCmdsBeginInfo);

    // For now, just supporting a single miplevel...
    int numMipLevels = 1;

    // Per miplevel values
    VkImage mipImages[numMipLevels];
    VkDeviceMemory  mipMemory[numMipLevels];
    uint32_t mipWidth[numMipLevels] = {textureObject->mWidth};
    uint32_t mipHeight[numMipLevels] = {textureObject->mHeight};

    // For each mip level
    for (uint32_t level = 0; level < numMipLevels; level++)
    {
        // Set the image size for this mip level
        imageCreateInfo.extent.width  = mipWidth[level];
        imageCreateInfo.extent.height = mipHeight[level];
        imageCreateInfo.extent.depth  = 1;

        // Create the mip level image
        err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, NULL, &mipImages[level]);
        assert(!err);

        vkGetImageMemoryRequirements(framework->GetDevice(), mipImages[level], &mem_reqs);

        memoryAllocateInfo.allocationSize   = mem_reqs.size;
        pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
        assert(pass);

        // allocate memory
        err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &mipMemory[level]);
        assert(!err);

        // bind memory
        err = vkBindImageMemory(framework->GetDevice(), mipImages[level], mipMemory[level], 0);
        assert(!err);

        // copy image data to the mip memory
        VkImageSubresource subRes = {};
        subRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        VkSubresourceLayout subResLayout;
        vkGetImageSubresourceLayout(framework->GetDevice(), mipImages[level], &subRes, &subResLayout);
        void *data;
        err = vkMapMemory(framework->GetDevice(), mipMemory[level], 0, mem_reqs.size, 0, &data);
        assert(!err);
        memcpy(data, imageData, mem_reqs.size);
        vkUnmapMemory(framework->GetDevice(), mipMemory[level]);

        // Change the mip image layout to transfer src
        framework->SetImageLayout(mipImages[level],
                               framework->GetSetupCommandBuffer(),
                               VK_IMAGE_ASPECT_COLOR_BIT,
                               VK_IMAGE_LAYOUT_PREINITIALIZED,
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                               VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, level, numMipLevels);
    }

    // Setup texture as blit target
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.mipLevels = numMipLevels;
    imageCreateInfo.extent = {textureObject->mWidth, textureObject->mHeight, 1 };

    // Create the image
    err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, nullptr, &textureObject->mImage);
    assert(!err);

    // Get the memory requirements
    vkGetImageMemoryRequirements(framework->GetDevice(), textureObject->mImage, &mem_reqs);

    // Get the memory type
    memoryAllocateInfo.allocationSize   = mem_reqs.size;
    pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // allocate memory
    err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &textureObject->mMem);
    assert(!err);

    // bind memory
    err = vkBindImageMemory(framework->GetDevice(), textureObject->mImage, textureObject->mMem, 0);
    assert(!err);

    // Change image layout to Transfer_DST so it can be filled.
    framework->SetImageLayout(textureObject->mImage,
                           framework->GetSetupCommandBuffer(),
                           VK_IMAGE_ASPECT_COLOR_BIT,
                           VK_IMAGE_LAYOUT_PREINITIALIZED,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, numMipLevels);

    // Copy mip levels
    for (uint32_t level = 0; level < numMipLevels; level++)
    {
        // Create a region for the image blit
        VkImageCopy region = {};

        region.srcSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.baseArrayLayer    = 0;
        region.srcSubresource.mipLevel          = 0;
        region.srcSubresource.layerCount        = 1;
        region.srcOffset                        = { 0, 0, 0 };
        region.dstSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstSubresource.baseArrayLayer    = 0;
        region.dstSubresource.mipLevel          = level;
        region.dstSubresource.layerCount        = 1;
        region.dstOffset                        = { 0, 0, 0 };
        region.extent.width                     = mipWidth[level];
        region.extent.height                    = mipHeight[level];
        region.extent.depth                     = 1;

        // Put image copy for this mip level into command buffer
        vkCmdCopyImage(framework->GetSetupCommandBuffer(),
                       mipImages[level],
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       textureObject->mImage,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    framework->SetImageLayout(textureObject->mImage,
                           framework->GetSetupCommandBuffer(),
                           VK_IMAGE_ASPECT_COLOR_BIT,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, numMipLevels);

    // We are finished recording operations.
    vkEndCommandBuffer(framework->GetSetupCommandBuffer());

    // Prepare to submit the command buffer
    VkCommandBuffer buffers[1];
    buffers[0] = framework->GetSetupCommandBuffer();

    VkSubmitInfo submit_info;
    submit_info.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext                   = NULL;
    submit_info.waitSemaphoreCount      = 0;
    submit_info.pWaitSemaphores         = NULL;
    submit_info.pWaitDstStageMask       = NULL;
    submit_info.commandBufferCount      = 1;
    submit_info.pCommandBuffers         = &buffers[0];
    submit_info.signalSemaphoreCount    = 0;
    submit_info.pSignalSemaphores       = NULL;

    // Submit to our shared graphics queue.
    err = vkQueueSubmit(framework->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    assert(!err);

    // Wait for the queue to become idle.
    err = vkQueueWaitIdle(framework->GetGraphicsQueue());
    assert(!err);

    // Cleanup the mip structures
    for (uint32_t level = 0; level < numMipLevels; level++)
    {
        vkDestroyImage(framework->GetDevice(), mipImages[level], nullptr);
        vkFreeMemory(  framework->GetDevice(), mipMemory[level], nullptr);
    }

    // Now create a sampler for this image, with required details
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType                     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext                     = nullptr;
    samplerCreateInfo.magFilter                 = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter                 = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode                = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias                = 0.0f;
    samplerCreateInfo.anisotropyEnable          = VK_FALSE;
    samplerCreateInfo.maxAnisotropy             = 0;
    samplerCreateInfo.compareOp                 = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod                    = 0.0f;
    samplerCreateInfo.maxLod                    = 0.0f;
    samplerCreateInfo.borderColor               = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates   = VK_FALSE;

    err = vkCreateSampler(framework->GetDevice(), &samplerCreateInfo, NULL, &textureObject->mSampler);
    assert(!err);

    // Create the image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext                            = NULL;
    viewCreateInfo.image                            = VK_NULL_HANDLE;
    viewCreateInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format                           = textureObject->mFormat;
    viewCreateInfo.components.r                     = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g                     = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b                     = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a                     = VK_COMPONENT_SWIZZLE_A;
    viewCreateInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel    = 0;
    viewCreateInfo.subresourceRange.levelCount      = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer  = 0;
    viewCreateInfo.subresourceRange.layerCount      = 1;
    viewCreateInfo.flags                            = 0;
    viewCreateInfo.image                            = textureObject->mImage;

    err = vkCreateImageView(framework->GetDevice(), &viewCreateInfo, NULL, &textureObject->mView);
    assert(!err);

    // All relevant objects created
    return true;
}

///////////////////////////////////////////////////////////////////////////////

bool TextureObject::FromASTCFile(VkFramework* framework, const char* filename, TextureObject* textureObject)
{
    if (!textureObject || !framework || !filename)
    {
        return false;
    }

    // Get the image file from the Android asset manager
    AAsset* file = AAssetManager_open(framework->GetAssetManager(), filename, AASSET_MODE_BUFFER);
    assert(file);

    uint32_t file_buffer_size = AAsset_getLength(file);
    uint8_t* file_buffer = ( uint8_t*)AAsset_getBuffer(file);

    assert(file_buffer);

    // The ASTC image assets have a basic header, followed by raw data.
    struct astc_header
    {
        uint8_t magic[4];
        uint8_t blockdim_x;
        uint8_t blockdim_y;
        uint8_t blockdim_z;
        uint8_t xsize[3];
        uint8_t ysize[3];
        uint8_t zsize[3];
    };

    astc_header* header = (astc_header*)file_buffer;
    uint8_t* astc_data = file_buffer + sizeof(astc_header);
    uint32_t astc_data_size = file_buffer_size - sizeof(astc_header);

    textureObject->mWidth   = header->xsize[0] + ((uint32_t)header->xsize[1]<<8) + ((uint32_t)header->xsize[2]<<16);
    textureObject->mHeight  = header->ysize[0] + ((uint32_t)header->ysize[1]<<8) + ((uint32_t)header->ysize[2]<<16);

    // Do not support 3d textures
    assert(header->blockdim_z==1);

    // We map the block dimensions to vulkan formats
    // More mappings can be added here. More information on supported formats can be found in the
    // Vulkan Specification
    if (header->blockdim_x == 8 && header->blockdim_y == 8)
    {
        textureObject->mFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    }
    else if (header->blockdim_x == 4 && header->blockdim_y == 4)
    {
        textureObject->mFormat = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    }
    else if (header->blockdim_x == 10 && header->blockdim_y == 10)
    {
        textureObject->mFormat = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    }
    else
    {
        LOGE("Whilst loading %s: Unsupported block dimensions %dx%d", filename, header->blockdim_x, header->blockdim_y);
        assert("Unsupported block dimensions"&&0);
    }

    VkResult   err;
    bool   pass;

    textureObject->mFramework = framework;


    // Initialize the Create Info structure
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext               = NULL;
    imageCreateInfo.imageType           = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format              = textureObject->mFormat;
    imageCreateInfo.extent.depth        = 1;
    imageCreateInfo.extent.height       = textureObject->mHeight;
    imageCreateInfo.extent.width        = textureObject->mWidth;
    imageCreateInfo.mipLevels           = 1;
    imageCreateInfo.arrayLayers         = 1;
    imageCreateInfo.samples             = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling              = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.usage               = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.flags               = 0;
    imageCreateInfo.sharingMode         = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout       = VK_IMAGE_LAYOUT_PREINITIALIZED;

    // Initialize the memory allocation structure
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext            = NULL;
    memoryAllocateInfo.allocationSize   = 0;
    memoryAllocateInfo.memoryTypeIndex  = 0;

    // Create a source image that will help us load the texture
    VkImage srcImage;
    err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, NULL, &srcImage);
    assert(!err);

    // Get the memory requirements for this source image
    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(framework->GetDevice(), srcImage, &mem_reqs);
    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // Allocate and bind the memory for the source image
    VkDeviceMemory srcMem;
    err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &srcMem);
    assert(!err);
    err = vkBindImageMemory(framework->GetDevice(), srcImage, srcMem, 0);
    assert(!err);

    // Map the memory and copy to it, and then unmap
    void *data;
    err = vkMapMemory(framework->GetDevice(), srcMem, 0, memoryAllocateInfo.allocationSize, 0, &data);
    assert(!err);
    assert(memoryAllocateInfo.allocationSize >= astc_data_size );
    memcpy( data, astc_data, astc_data_size);
    vkUnmapMemory(framework->GetDevice(), srcMem);

    // Setup texture as blit (destination) target ( and also to be subsequently used as a sampled texture
    imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage       = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.mipLevels   = 1;
    imageCreateInfo.extent = {textureObject->mWidth, textureObject->mHeight, 1 };

    // Create the image
    err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, nullptr, &textureObject->mImage);
    assert(!err);

    // Get the memory requirements for the image
    vkGetImageMemoryRequirements(framework->GetDevice(), textureObject->mImage, &mem_reqs);
    memoryAllocateInfo.allocationSize   = mem_reqs.size;
    pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    // allocate and bind memory for the image
    err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &textureObject->mMem);
    assert(!err);
    err = vkBindImageMemory(framework->GetDevice(), textureObject->mImage, textureObject->mMem, 0);
    assert(!err);

    // Reset the setup command buffer
    vkResetCommandBuffer(framework->GetSetupCommandBuffer(), 0);

    VkCommandBufferInheritanceInfo commandBufferInheritanceInfo = {};
    commandBufferInheritanceInfo.sType                  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    commandBufferInheritanceInfo.pNext                  = NULL;
    commandBufferInheritanceInfo.renderPass             = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.subpass                = 0;
    commandBufferInheritanceInfo.framebuffer            = VK_NULL_HANDLE;
    commandBufferInheritanceInfo.occlusionQueryEnable   = VK_FALSE;
    commandBufferInheritanceInfo.queryFlags             = 0;
    commandBufferInheritanceInfo.pipelineStatistics     = 0;

    VkCommandBufferBeginInfo setupCmdsBeginInfo;
    setupCmdsBeginInfo.sType                            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    setupCmdsBeginInfo.pNext                            = NULL;
    setupCmdsBeginInfo.flags                            = 0;
    setupCmdsBeginInfo.pInheritanceInfo = &commandBufferInheritanceInfo;

    // Begin recording to the command buffer.
    vkBeginCommandBuffer(framework->GetSetupCommandBuffer(), &setupCmdsBeginInfo);

    // Change image layout to Transfer_DST so it can be filled.
    framework->SetImageLayout(textureObject->mImage, framework->GetSetupCommandBuffer(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 1);

    // Change source image layout to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL.
    framework->SetImageLayout(srcImage, framework->GetSetupCommandBuffer(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 1);

    // Create a region for the image blit
    VkImageCopy region = {};

    region.srcSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.baseArrayLayer    = 0;
    region.srcSubresource.mipLevel          = 0;
    region.srcSubresource.layerCount        = 1;
    region.srcOffset                        = { 0, 0, 0 };
    region.dstSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.baseArrayLayer    = 0;
    region.dstSubresource.mipLevel          = 0;
    region.dstSubresource.layerCount        = 1;
    region.dstOffset                        = { 0, 0, 0 };
    region.extent.width                     = textureObject->mWidth;
    region.extent.height                    = textureObject->mHeight;
    region.extent.depth                     = 1;

    // Put image copy into command buffer
    vkCmdCopyImage(framework->GetSetupCommandBuffer(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textureObject->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Change the layout of the image to shader read only
    framework->SetImageLayout(textureObject->mImage, framework->GetSetupCommandBuffer(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 1);

    // We are finished recording operation
    vkEndCommandBuffer(framework->GetSetupCommandBuffer());

    // Prepare to submit the command buffer
    VkCommandBuffer buffers[1];
    buffers[0] = framework->GetSetupCommandBuffer();

    VkSubmitInfo submit_info;
    submit_info.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext                   = NULL;
    submit_info.waitSemaphoreCount      = 0;
    submit_info.pWaitSemaphores         = NULL;
    submit_info.pWaitDstStageMask       = NULL;
    submit_info.commandBufferCount      = 1;
    submit_info.pCommandBuffers         = &buffers[0];
    submit_info.signalSemaphoreCount    = 0;
    submit_info.pSignalSemaphores       = NULL;

    // Submit to our shared graphics queue.
    err = vkQueueSubmit(framework->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    assert(!err);

    // Wait for the queue to become idle.
    err = vkQueueWaitIdle(framework->GetGraphicsQueue());
    assert(!err);

    // Cleanup
    vkDestroyImage(framework->GetDevice(), srcImage, nullptr);
    vkFreeMemory(framework->GetDevice(), srcMem, nullptr);

    // Update the layout so we remember it..
    textureObject->mImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Now create a sampler for this image, with required details
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType                         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext                         = nullptr;
    samplerCreateInfo.magFilter                     = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter                     = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode                    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU                  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV                  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW                  = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias                    = 0.0f;
    samplerCreateInfo.anisotropyEnable              = VK_FALSE;
    samplerCreateInfo.maxAnisotropy                 = 0;
    samplerCreateInfo.compareOp                     = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod                        = 0.0f;
    samplerCreateInfo.maxLod                        = 0.0f;
    samplerCreateInfo.borderColor                   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates       = VK_FALSE;
    err = vkCreateSampler(framework->GetDevice(), &samplerCreateInfo, NULL, &textureObject->mSampler);
    assert(!err);

    // Create the image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext                            = NULL;
    viewCreateInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format                           = textureObject->mFormat;
    viewCreateInfo.components.r                     = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g                     = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b                     = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a                     = VK_COMPONENT_SWIZZLE_A;
    viewCreateInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel    = 0;
    viewCreateInfo.subresourceRange.levelCount      = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer  = 0;
    viewCreateInfo.subresourceRange.layerCount      = 1;
    viewCreateInfo.flags                            = 0;
    viewCreateInfo.image                            = textureObject->mImage;

    err = vkCreateImageView(framework->GetDevice(), &viewCreateInfo, NULL, &textureObject->mView);
    assert(!err);

    AAsset_close(file);

    // All relevant objects created
    return true;
}

bool TextureObject::CreateTexture(VkFramework* framework, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags additionalUsage, TextureObject* textureObject)
{
    if (!textureObject || !framework)
    {
        return false;
    }

    textureObject->mFormat      = format;
    textureObject->mFramework   = framework;
    textureObject->mWidth       = width;
    textureObject->mHeight      = height;
    textureObject->mImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkResult   err;
    bool   pass;

    //textureObject->mFramework = framework;

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType               = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.pNext               = NULL;
    imageCreateInfo.imageType           = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format              = textureObject->mFormat;
    imageCreateInfo.extent.depth        = 1.0f;
    imageCreateInfo.extent.height       = textureObject->mHeight;
    imageCreateInfo.extent.width        = textureObject->mWidth;
    imageCreateInfo.mipLevels           = 1;
    imageCreateInfo.arrayLayers         = 1;
    imageCreateInfo.samples             = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling              = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage               = VK_IMAGE_USAGE_SAMPLED_BIT | additionalUsage;
    imageCreateInfo.flags               = 0;

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext            = NULL;
    memoryAllocateInfo.allocationSize   = 0;
    memoryAllocateInfo.memoryTypeIndex  = 0;

    VkMemoryRequirements mem_reqs;

    err = vkCreateImage(framework->GetDevice(), &imageCreateInfo, NULL, &textureObject->mImage);
    assert(!err);

    vkGetImageMemoryRequirements(framework->GetDevice(), textureObject->mImage, &mem_reqs);

    memoryAllocateInfo.allocationSize  = mem_reqs.size;
    pass = framework->GetMemoryTypeFromProperties( mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);
    assert(pass);

    err = vkAllocateMemory(framework->GetDevice(), &memoryAllocateInfo, NULL, &textureObject->mMem);
    assert(!err);

    err = vkBindImageMemory(framework->GetDevice(), textureObject->mImage, textureObject->mMem, 0);
    assert(!err);

    // Change the layout of the image to shader read only
    textureObject->SetLayoutImmediate(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, textureObject->mImageLayout );

    // Now create a sampler for this image, with required details
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType                     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext                     = nullptr;
    samplerCreateInfo.magFilter                 = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter                 = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode                = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias                = 0.0f;
    samplerCreateInfo.anisotropyEnable          = VK_FALSE;
    samplerCreateInfo.maxAnisotropy             = 0;
    samplerCreateInfo.compareOp                 = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod                    = 0.0f;
    samplerCreateInfo.maxLod                    = 0.0f;
    samplerCreateInfo.borderColor               = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.unnormalizedCoordinates   = VK_FALSE;

    err = vkCreateSampler(framework->GetDevice(), &samplerCreateInfo, NULL, &textureObject->mSampler);
    assert(!err);

    // Create the image view
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.pNext                            = NULL;
    viewCreateInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format                           = textureObject->mFormat;
    viewCreateInfo.components.r                     = VK_COMPONENT_SWIZZLE_R;
    viewCreateInfo.components.g                     = VK_COMPONENT_SWIZZLE_G;
    viewCreateInfo.components.b                     = VK_COMPONENT_SWIZZLE_B;
    viewCreateInfo.components.a                     = VK_COMPONENT_SWIZZLE_A;
    viewCreateInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    viewCreateInfo.subresourceRange.baseMipLevel    = 0;
    viewCreateInfo.subresourceRange.levelCount      = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer  = 0;
    viewCreateInfo.subresourceRange.layerCount      = 1;
    viewCreateInfo.flags                            = 0;
    viewCreateInfo.image                            = textureObject->mImage;

    err = vkCreateImageView(framework->GetDevice(), &viewCreateInfo, NULL, &textureObject->mView);
    assert(!err);

    // All relevant objects created
    return true;
}

#endif

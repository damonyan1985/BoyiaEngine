#ifndef TextureObject_h
#define TextureObject_h

#include <assert.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <android/asset_manager.h>

class VkFramework;

// Defines a simple object for creating and holding Vulkan image/view objects.
class ImageViewObject
{
public:
    ImageViewObject() : mWidth(0), mHeight(0), mSample(nullptr), mImage(VK_NULL_HANDLE), mMem(VK_NULL_HANDLE), mView(VK_NULL_HANDLE) {}
    ~ImageViewObject();

    static bool CreateImageView(VkFramework* framework, uint32_t width, uint32_t height, VkFormat format, VkImageAspectFlags aspect,
                                VkImageLayout layout, VkImageUsageFlags usage, VkFlags memoryRequirmentFlags, ImageViewObject* textureObject);

    void SetLayoutImmediate(VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout);

    virtual bool Destroy();

    VkImageView GetView()
    {
        return mView;
    }

    VkImage GetImage()
    {
        return mImage;
    }

    VkImageLayout GetLayout()
    {
        return mImageLayout;
    }

    void SetFormat(VkFormat format)
    {
        mFormat = format;
    }

    VkFormat GetFormat()
    {
        return mFormat;
    }

    void SetWidth(uint32_t width)
    {
        mWidth = width;
    }

    uint32_t GetWidth()
    {
        return mWidth;
    }

    void SetHeight(uint32_t height)
    {
        mHeight = height;
    }

    uint32_t GetHeight()
    {
        return mHeight;
    }

protected:
    VkImage mImage;
    VkImageLayout mImageLayout;
    VkDeviceMemory mMem;
    VkImageView mView;
    VkFramework* mFramework;

    VkFormat mFormat;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mNumMipLevels;
    uint32_t mNumFaces;

};

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#define GL_TEXTURE_1D           0x0DE0
#define GL_TEXTURE_2D           0x0DE1
#define GL_TEXTURE_3D           0x806F
#define GL_TEXTURE_CUBE_MAP     0x8513

#define GL_RGB                                  0x1907
#define GL_RGBA                                 0x1908
#define GL_COMPRESSED_RGB8_ETC2                 0x9274
#define GL_COMPRESSED_RGBA8_ETC2_EAC            0x9278
#define GL_COMPRESSED_SRGB8_ETC2                0x9275
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC     0x9279
#define GL_RGBA16F                              0x881A
#define GL_SRGB8_ALPHA8                         0x8C43
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD          0x8C93

#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }
#define KTX_ENDIAN_REF      (0x04030201)
#define KTX_ENDIAN_REF_REV  (0x01020304)
#define KTX_HEADER_SIZE		(64)

typedef enum
{
    KTX_SUCCESS = 0,
    KTX_HEADER_ERROR,
    KTX_UNEXPECTED_END_OF_STREAM,
    KTX_INVALID_VALUE,
    KTX_UNSUPPORTED_TEXTURE_TYPE,
    KTX_GL_ERROR,
} TKTXErrorCode;

typedef struct
{
    uint8_t   identifier[12];
    uint32_t   endianness;
    uint32_t   glType;
    uint32_t   glTypeSize;
    uint32_t   glFormat;
    uint32_t   glInternalFormat;
    uint32_t   glBaseInternalFormat;
    uint32_t   pixelWidth;
    uint32_t   pixelHeight;
    uint32_t   pixelDepth;
    uint32_t   numberOfArrayElements;
    uint32_t   numberOfFaces;
    uint32_t   numberOfMipmapLevels;
    uint32_t   bytesOfKeyValueData;
} TKTXHeader;

typedef struct
{
    uint32_t  nTextureDimensions;
    uint32_t  glTarget;
    bool      bCompressed;
} TKTXTextureInfo;


// Defines a simple object for creating and holding Vulkan texture objects.
// Supports loading from TGA files in Android Studio asset folder.
// Only supports R8G8B8A8 files and texture formats. Converts from TGA BGRA to RGBA.
class TextureObject: public ImageViewObject
{
public:
    TextureObject();
    ~TextureObject();

    static bool FromKTXFile(VkFramework* sample, const char* filename, TextureObject* pTextureObject);

    static bool FromTGAFile(VkFramework* sample, const char* filename, TextureObject* textureObject);

    static bool FromTGAImageData(VkFramework *sample, TextureObject *textureObject, uint32_t *imageData);

    static bool FromASTCFile(VkFramework* sample, const char* filename, TextureObject* textureObject);


    static bool CreateTexture(VkFramework* sample, uint32_t width, uint32_t height,
                                             VkFormat format, VkImageUsageFlags additionalUsage,
                                             TextureObject* textureObject);

    VkSampler GetSampler()
    {
        return mSampler;
    }

    bool Destroy();



protected:
    static TKTXErrorCode ParseHeader(TKTXHeader* pHeader, TKTXTextureInfo* pTextureInfo);
    static TKTXErrorCode LoadKTXFromMemory(VkFramework* sample, const uint8_t* dataInMemory, uint32_t filesize, TextureObject* textureObject);


    static uint8_t* LoadTGAFromMemory(const uint8_t* dataInMemory, uint32_t* pWidth, uint32_t* pHeight, VkFormat* pFormat);

    static uint32_t StreamRead(void* pData, unsigned int nSize);
    static uint32_t StreamSkip(unsigned int nSize);

    VkSampler mSampler;

    const static uint8_t* mpStreamBuffer;
    static uint32_t mnStreamBufferIndex;
    static uint32_t mnStreamBufferSize;
};

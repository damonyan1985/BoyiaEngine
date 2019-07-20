#ifndef VkFramework_h
#define VkFramework_h


// This definition enables the Android extensions
#define VK_USE_PLATFORM_ANDROID_KHR
#define VK_PROTOTYPES

#define ANDROID_TAG "VkFramework"

#include "PlatformLib.h"
#include "native_app_glue/android_native_app_glue.h"
#include <assert.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <android/log.h>
#include <android/asset_manager.h>

// Include Memory Manager
#include "MemoryAllocator.h"

//Include the texture object helper
#include "TextureObject.h"

//BufferObject helper classes
#include "BufferObject.h"

// Thread
#include "Thread.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  ANDROID_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  ANDROID_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, ANDROID_TAG, __VA_ARGS__))

// Helper define for tracking error locations in code
#define VK_CHECK(X) if (!(X)) { LOGE("VK_CHECK Failure"); assert((X));}

#define WAIT_FOREVER 0xFFFFFFFFFFFFFFFF
#define MICRO_TO_UNIT (1.0f/1000000000.0f)

struct DepthBuffer
{
    VkFormat format;
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct SwapchainBuffer
{
    VkImage image;
    VkCommandBuffer cmdBuffer;
    VkImageView view;
};

class VkFramework
{
public:
    VkFramework(const char* name, uint32_t version);
    VkFramework();
    ~VkFramework();

    bool Initialize(ANativeWindow* window);

    void ShutDown();

    void DrawFrame();

    bool IsInitialized()
    {
        return mInitialized;
    }

    bool TearDown();

    AAssetManager *GetAssetManager()
    {
        return mAssetManager;
    }

    VkDevice GetDevice()
    {
        return mDevice;
    }

    VkCommandBuffer GetSetupCommandBuffer()
    {
        return mSetupCommandBuffer;
    }

    VkQueue GetGraphicsQueue()
    {
        return mQueue;
    }

    void SetAssetManager(AAssetManager* manager)
    {
        mAssetManager = manager;
    }

    bool GetMemoryTypeFromProperties( uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

    bool GetUsingMemoryAllocator()
    {
        return mUseMemoryAllocator;
    }

    void SetUseMemoryAllocator(bool useMemoryAllocator)
    {
        mUseMemoryAllocator = useMemoryAllocator;
    }

    MemoryAllocator* GetMemoryAllocator()
    {
        return mMemoryAllocator;
    }

    void SetUseValidation(bool useValidation)
    {
        mUseValidation = useValidation;
    }

    uint32_t GetWidth()
    {
        return mWidth;
    }

    uint32_t GetHeight()
    {
        return mHeight;
    }

    uint32_t GetSwapChainCount()
    {
        return mSwapchainImageCount;
    }

    uint32_t GetSwapChainIndex()
    {
        return mSwapchainCurrentIdx;
    }

    // Image Layout helper
    void SetImageLayout(VkImage image, VkCommandBuffer cmdBuffer, VkImageAspectFlags aspect, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcMask=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkPipelineStageFlags dstMask=VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, uint32_t mipLevel=0, uint32_t mipLevelCount=1);

    // File Reader
    bool LoadFileBuffer(const char* pszFileName, void** ppBuffer, uint32_t* pBufferSize);
    bool SaveFileBuffer(const char* pszFileName, void* pBuffer, uint32_t bufferSize);

    // Shader Modules
    VkShaderModule CreateShaderModuleFromAsset(const char* asset);
    VkShaderModule CreateShaderModule(const uint32_t* code, uint32_t size);


    // This version of InitPipeline requires various state, but will provide default versions of
    // some Info structures if nullptr is passed.
    void InitPipeline(VkPipelineCache							    pipelineCache,
                      uint32_t                                      stageCount,
                      const VkPipelineShaderStageCreateInfo*        stages,
                      VkPipelineLayout                              layout,
                      VkRenderPass                                  renderPass,
                      uint32_t                                      subpass,
                      const VkPipelineVertexInputStateCreateInfo*   vertexInputState,
            // info structures below can be nullptr, defaults will be provided
                      const VkPipelineInputAssemblyStateCreateInfo* inputAssemblyState,
                      const VkPipelineTessellationStateCreateInfo*  tessellationState,
                      const VkPipelineViewportStateCreateInfo*      viewportState,
                      const VkPipelineRasterizationStateCreateInfo* rasterizationState,
                      const VkPipelineMultisampleStateCreateInfo*   multisampleState,
                      const VkPipelineDepthStencilStateCreateInfo*  depthStencilState,
                      const VkPipelineColorBlendStateCreateInfo*    colorBlendState,
                      const VkPipelineDynamicStateCreateInfo*       dynState,
                      bool                                          bAllowDerivation,
                      VkPipeline                                    deriveFromPipeline,
            // pipeline object to store created handle
                      VkPipeline*                                   pipeline);

    void InitPipeline(VkPipelineCache                               pipelineCache,
                      VkPipelineVertexInputStateCreateInfo*         visci,
                      VkPipelineLayout                              pipelineLayout,
                      VkRenderPass                                  renderPass,
                      VkPipelineRasterizationStateCreateInfo*       providedRS,
                      VkPipelineDepthStencilStateCreateInfo*        providedDS,
                      VkShaderModule                                vertShaderModule,
                      VkShaderModule                                fragShaderModule,
                      bool                                          bAllowDerivation,
                      VkPipeline                                    deriveFromPipeline,
                      VkPipeline* pipeline);

    uint32_t GetQueueFamilyIndex(){return mQueueFamilyIndex;};
    uint32_t GetFrameIndex(){return mFrameIdx;};

    // Pointer
    virtual void PointerDownEvent(uint32_t iPointerID, float xPos, float yPos);
    virtual void PointerUpEvent(uint32_t iPointerID, float xPos, float yPos);
    virtual void PointerMoveEvent(uint32_t iPointerID, float xPos, float yPos);


    // FPS
    void LogFPS(bool bLog){mbLogFPS = bLog;};

protected:
    void InitializeValues();

    bool CreateInstance();
    bool GetPhysicalDevices();
    void InitSurface();
    void InitDevice();
    void InitSwapchain();
    void InitCommandbuffers();
    void InitSync();
    void InitSwapchainLayout();

    void SetNextBackBuffer();
    void PresentBackBuffer();

    // Sample overrides
    virtual bool InitSample()                                   = 0;
    virtual bool Update()                                       = 0;
    virtual bool Draw()                                         = 0;
    virtual bool DestroySample()                                = 0;
    virtual void WindowResize(uint32_t width, uint32_t height)  = 0;



    void TargetToTexture(VkCommandBuffer commandBuffer, ImageViewObject& sourceTarget, TextureObject& destTexture);

    // Instance Extensions
    bool CheckInstanceExtensions();
    int mEnabledInstanceExtensionCount;
    const char* mInstanceExtensionNames[16];

    // Instance Layers
    bool CheckInstanceLayerValidation();
    static const char*  mInstanceLayers[];
    int32_t mEnabledInstanceLayerCount;

    // Validation Callbacks
    void CreateValidationCallbacks();
    PFN_vkCreateDebugReportCallbackEXT  mCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT mDestroyDebugReportCallbackEXT;
    PFN_vkDebugReportMessageEXT         mDebugReportMessageCallback;
    VkDebugReportCallbackEXT            mDebugReportCallback;

    // Android objects
    ANativeWindow* mAndroidWindow;
    AAssetManager* mAssetManager;

    // Vulkan objects
    VkInstance mInstance;
    VkPhysicalDevice* mpPhysicalDevices;
    VkPhysicalDevice mPhysicalDevice;
    VkPhysicalDeviceProperties mPhysicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties mPhysicalDeviceMemoryProperties;
    VkDevice mDevice;
    uint32_t mPhysicalDeviceCount;
    uint32_t mQueueFamilyIndex;
    VkQueue mQueue;
    VkSurfaceKHR mSurface;
    VkSurfaceFormatKHR mSurfaceFormat;
    VkCommandPool mCommandPool;

    // This shared cmdBuffer is used for setup operations on e.g. textures
    VkCommandBuffer mSetupCommandBuffer;

    // Swapchain
    VkSwapchainKHR mSwapchain;
    SwapchainBuffer* mSwapchainBuffers;
    uint32_t mSwapchainCurrentIdx;
    uint32_t mSwapchainImageCount;
    DepthBuffer* mDepthBuffers;

    // Vulkan Synchronization objects
    VkSemaphore mBackBufferSemaphore;
    VkSemaphore mRenderCompleteSemaphore;
    VkFence mFence;

    // Render size
    uint32_t mHeight;
    uint32_t mWidth;

    // Time
    uint64_t GetTime();
    uint64_t mFrameTimeBegin;
    uint64_t mFrameTimeEnd;
    uint64_t mFrameTimeDelta;
    uint64_t mFrameIdx;

    // Logging
    bool mbLogFPS;

    // State
    bool mInitialized;
    bool mInitBegun;

    // Sample version and name
    uint32_t mSampleVersion;
    const char* mSampleName;

    // Memory Allocation
    MemoryAllocator*mMemoryAllocator;
    bool mUseMemoryAllocator;

    // Validation
    bool mUseValidation;

    // Touch
    uint32_t mTouchX;
    uint32_t mTouchY;
    bool mTouchDown;
};

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#编译器优化会导致一些问题
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_MODULE    := Boyia
LOCAL_ARM_MODE  := arm
LOCAL_CFLAGS    := -Werror
LOCAL_CFLAGS    += -DFT2_BUILD_LIBRARY=1 -DANDROID_NDK
#LOCAL_CFLAGS    += -DMINI_VULKAN
LOCAL_LDLIBS    := -llog -lEGL -lGLESv3 -landroid -ljnigraphics
#LOCAL_STATIC_LIBRARIES := libcurl
LOCAL_SHARED_LIBRARIES := libcurl libjpeg libpng libft2
#LOCAL_LDLIBS    += -lvulkan
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_CPP_EXTENSION := .cpp
LOCAL_DISABLE_FORMAT_STRING_CHECKS := true
# platform instance by gl
# LOCAL_LDLIBS := -llog -lGLESv1_CM -ldl
# draw api instance

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/framework/style \
    $(LOCAL_PATH)/framework/gui \
    $(LOCAL_PATH)/framework/widget \
    $(LOCAL_PATH)/framework/widget/flex \
    $(LOCAL_PATH)/framework/loader \
    $(LOCAL_PATH)/framework/dom \
    $(LOCAL_PATH)/framework/util \
    $(LOCAL_PATH)/framework/view \
    $(LOCAL_PATH)/framework/platform \
    $(LOCAL_PATH)/framework/platform/arm \
    $(LOCAL_PATH)/framework/platform/android \
    $(LOCAL_PATH)/framework/platform/graphics \
    $(LOCAL_PATH)/framework/utils \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/framework/byvm/core \
    $(LOCAL_PATH)/framework/byvm/json \
    $(LOCAL_PATH)/framework/byvm/lib \
    $(LOCAL_PATH)/framework/byvm/binds \
    $(LOCAL_PATH)/framework/byvm/jit \
    $(LOCAL_PATH)/framework/byvm/app \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/network \
    $(LOCAL_PATH)/network/image \
    $(LOCAL_PATH)/network/font \
    $(LOCAL_PATH)/network/threadpool \

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/graphics/core \
    $(LOCAL_PATH)/graphics/shaders \
    $(LOCAL_PATH)/graphics/painter \
    $(LOCAL_PATH)/graphics/vulkan \
    
LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../External/curl/include \
    $(LOCAL_PATH)/../External/jpeg \
    $(LOCAL_PATH)/../External/lpng/include \
    $(LOCAL_PATH)/../External/ft2/include \
    $(LOCAL_PATH)/../External/trace/ \

include $(LOCAL_PATH)/framework/Android.fw.util.mk

include $(LOCAL_PATH)/framework/Android.fw.mk

include $(LOCAL_PATH)/graphics/Android.graphics.mk

include $(LOCAL_PATH)/network/Android.network.mk

include $(LOCAL_PATH)/framework/Android.by.mk

LOCAL_SRC_FILES += \
    $(LOCAL_PATH)/porting/BoyiaJniOnLoad.cpp \
    $(LOCAL_PATH)/porting/BoyiaViewCore.cpp

include $(BUILD_SHARED_LIBRARY)
    
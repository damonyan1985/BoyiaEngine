LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

NDK_PROJECT_PATH := $(LOCAL_PATH)/../
LOCAL_C_INCLUDES:= \
					$(SYSROOT)/system/core/include \
					$(NDK_PROJECT_PATH)/include \
					$(NDK_PROJECT_PATH)/include/openssl 
					
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
LOCAL_SHARED_LIBRARIES := libssl libcrypto

LOCAL_MODULE    := encryption
LOCAL_SRC_FILES := encryption.c

include $(BUILD_SHARED_LIBRARY)

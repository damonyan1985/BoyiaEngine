LOCAL_PATH:= $(call my-dir)

# We need to build this for both the device (as a shared library)
# and the host (as a static library for tools to use).

include $(CLEAR_VARS)
#LOCAL_SRC_FILES := $(addprefix src/,$(CSOURCES))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/

LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/src/png.c \
	$(LOCAL_PATH)/src/pngerror.c \
	$(LOCAL_PATH)/src/pngget.c \
	$(LOCAL_PATH)/src/pngmem.c \
	$(LOCAL_PATH)/src/pngpread.c \
	$(LOCAL_PATH)/src/pngread.c \
	$(LOCAL_PATH)/src/pngrio.c \
	$(LOCAL_PATH)/src/pngrtran.c \
	$(LOCAL_PATH)/src/pngrutil.c \
	$(LOCAL_PATH)/src/pngset.c \
	$(LOCAL_PATH)/src/pngtrans.c \
	$(LOCAL_PATH)/src/pngwio.c \
	$(LOCAL_PATH)/src/pngwrite.c \
	$(LOCAL_PATH)/src/pngwtran.c \
	$(LOCAL_PATH)/src/pngwutil.c \		

common_CFLAGS := -std=gnu89 -Wno-unused-parameter
LOCAL_CLANG := true
#LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS) -ftrapv
#LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := libz
LOCAL_MODULE:= png
include $(BUILD_SHARED_LIBRARY)
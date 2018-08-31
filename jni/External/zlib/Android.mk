LOCAL_PATH:= $(call my-dir)

# We need to build this for both the device (as a shared library)
# and the host (as a static library for tools to use).

include $(CLEAR_VARS)
#LOCAL_SRC_FILES := $(addprefix src/,$(CSOURCES))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/

LOCAL_SRC_FILES += \
	$(LOCAL_PATH)/src/adler32.c \
	$(LOCAL_PATH)/src/compress.c \
	$(LOCAL_PATH)/src/crc32.c \
	$(LOCAL_PATH)/src/deflate.c \
	$(LOCAL_PATH)/src/gzclose.c \
	$(LOCAL_PATH)/src/gzlib.c \
	$(LOCAL_PATH)/src/gzread.c \
	$(LOCAL_PATH)/src/gzwrite.c \
	$(LOCAL_PATH)/src/infback.c \
	$(LOCAL_PATH)/src/inffast.c \
	$(LOCAL_PATH)/src/inflate.c \
	$(LOCAL_PATH)/src/inftrees.c \
	$(LOCAL_PATH)/src/trees.c \
	$(LOCAL_PATH)/src/uncompr.c \
	$(LOCAL_PATH)/src/zutil.c \

common_CFLAGS := -std=gnu89 -Wno-unused-parameter
LOCAL_CLANG := true
#LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS) -ftrapv
LOCAL_MODULE:= z
include $(BUILD_SHARED_LIBRARY)
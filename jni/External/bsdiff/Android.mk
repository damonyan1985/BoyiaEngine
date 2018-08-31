LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_MODULE := bsdiff
LOCAL_SRC_FILES += \
    $(LOCAL_PATH)/source/blocksort.c \
    $(LOCAL_PATH)/source/bzlib.c \
    $(LOCAL_PATH)/source/compress.c \
    $(LOCAL_PATH)/source/crctable.c \
    $(LOCAL_PATH)/source/decompress.c \
    $(LOCAL_PATH)/source/huffman.c \
    $(LOCAL_PATH)/source/randtable.c \
    $(LOCAL_PATH)/source/bsdiff.c \
    $(LOCAL_PATH)/source/bspatch.c \
    $(LOCAL_PATH)/source/bsdiffjni.c
    
include $(BUILD_SHARED_LIBRARY)
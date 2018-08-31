LOCAL_PATH:= $(call my-dir)
  
  
common_CFLAGS := -Wpointer-arith -Wwrite-strings -Wunused -Winline \
                -Wnested-externs -Wmissing-declarations -Wmissing-prototypes \
                -Wno-long-long -Wfloat-equal -Wno-multichar -Wsign-compare \
                -Wno-format-nonliteral -Wendif-labels -Wstrict-prototypes \
                -Wdeclaration-after-statement \
                -Wno-system-headers -DHAVE_CONFIG_H -DBUILDING_LIBCURL
  
  
#########################  
# Build the libcurl library  
  
  
include $(CLEAR_VARS)
include $(LOCAL_PATH)/lib/Makefile.inc
CURL_HEADERS := \
    curlbuild.h \
    curl.h \
    curlrules.h \
    curlver.h \
    easy.h \
    mprintf.h \
    multi.h \
    stdcheaders.h \
    typecheck-gcc.h
  
  
LOCAL_SRC_FILES := $(addprefix lib/,$(CSOURCES))
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/lib/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ssl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../ssl/crypto
LOCAL_CFLAGS += $(common_CFLAGS)


LOCAL_COPY_HEADERS_TO := $(LOCAL_PATH)/libcurl/curl
LOCAL_COPY_HEADERS := $(addprefix include/curl/,$(CURL_HEADERS))


LOCAL_MODULE:= libcurl
LOCAL_MODULE_TAGS := optional
  
LOCAL_SHARED_LIBRARIES := libssl libcrypto
# Copy the licence to a place where Android will find it.  
# Actually, this doesn't quite work because the build system searches  
# for NOTICE files before it gets to this point, so it will only be seen  
# on subsequent builds.  
  
  
include $(BUILD_SHARED_LIBRARY)
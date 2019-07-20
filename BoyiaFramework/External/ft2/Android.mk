LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)


LOCAL_MODULE := ft2


LOCAL_CFLAGS := -DANDROID_NDK \
  -DFT2_BUILD_LIBRARY=1


LOCAL_C_INCLUDES := $(LOCAL_PATH)/include \


LOCAL_SRC_FILES := \
 source/autofit/autofit.c \
 source/base/basepic.c \
 source/base/ftapi.c \
 source/base/ftbase.c \
 source/base/ftbbox.c \
 source/base/ftbitmap.c \
 source/base/ftdbgmem.c \
 source/base/ftdebug.c \
 source/base/ftglyph.c \
 source/base/ftinit.c \
 source/base/ftpic.c \
 source/base/ftstroke.c \
 source/base/ftsynth.c \
 source/base/ftsystem.c \
 source/cff/cff.c \
 source/pshinter/pshinter.c \
 source/pshinter/pshglob.c \
 source/pshinter/pshpic.c \
 source/pshinter/pshrec.c \
 source/psnames/psnames.c \
 source/psnames/pspic.c \
 source/raster/raster.c \
 source/raster/rastpic.c \
 source/sfnt/pngshim.c \
 source/sfnt/sfntpic.c \
 source/sfnt/ttbdf.c \
 source/sfnt/ttkern.c \
 source/sfnt/ttload.c \
 source/sfnt/ttmtx.c \
 source/sfnt/ttpost.c \
 source/sfnt/ttsbit.c \
 source/sfnt/sfobjs.c \
 source/sfnt/ttcmap.c \
 source/sfnt/sfdriver.c \
 source/smooth/smooth.c \
 source/smooth/ftspic.c \
 source/truetype/truetype.c \
 source/type1/t1driver.c \
 source/cid/cidgload.c \
 source/cid/cidload.c \
 source/cid/cidobjs.c \
 source/cid/cidparse.c \
 source/cid/cidriver.c \
 source/pfr/pfr.c \
 source/pfr/pfrgload.c \
 source/pfr/pfrload.c \
 source/pfr/pfrobjs.c \
 source/pfr/pfrsbit.c \
 source/type42/t42objs.c \
 source/type42/t42parse.c \
 source/type42/type42.c \
 source/winfonts/winfnt.c \
 source/pcf/pcfread.c \
 source/pcf/pcfutil.c \
 source/pcf/pcfdrivr.c \
 source/psaux/afmparse.c \
 source/psaux/psaux.c \
 source/psaux/psconv.c \
 source/psaux/psobjs.c \
 source/psaux/t1decode.c \
 source/tools/apinames.c \
 source/type1/t1afm.c \
 source/type1/t1gload.c \
 source/type1/t1load.c \
 source/type1/t1objs.c \
 source/type1/t1parse.c\
 source/bdf/bdfdrivr.c\
 source/bdf/bdflib.c\
 source/gzip/ftgzip.c\
 source/lzw/ftlzw.c \




LOCAL_LDLIBS := -ldl -llog


include $(BUILD_SHARED_LIBRARY)
APP_PLATFORM := android-19
APP_ABI      := arm64-v8a
#APP_ABI      := armeabi-v7a
#APP_ABI      := armeabi-v7a
APP_SHORT_COMMANDS := true
APP_CFLAGS   += -Wno-error=format-security
APP_CPPFLAGS := -frtti -std=c++11
APP_STL      := stlport_static
NDK_TOOLCHAIN_VERSION=4.9
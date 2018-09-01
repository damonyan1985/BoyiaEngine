/*
============================================================================
 Name        : KLogInFile.h
 Author      : yanbo
 Version     :
 Copyright   : All Copyright Reserved
 Description : KLogInFile.h - header file
============================================================================
*/

#ifndef SalLog_h
#define SalLog_h

#include "PlatformLib.h"

#if ENABLE(ANDROID_PORTING)
#include <android/log.h>
#endif

#define  LOG_TAG    "libmini"
extern bool JNI_LOG_ON;

#if ENABLE(BROWSER_KERNEL)
#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(log_str)
#define KDESLOG(log_str)
#define KSTRLOG(log_str)
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...)
#define KSTRFORMAT(format_str, str)

#elif ENABLE(ANDROID_PORTING)
#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(...) if (JNI_LOG_ON) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define KDESLOG(log_str)
#define KSTRLOG(log_str) if (JNI_LOG_ON) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, (const char*)log_str.GetBuffer())
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...) if (JNI_LOG_ON) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format_str, __VA_ARGS__)
#define KSTRFORMAT(format_str, str) if (JNI_LOG_ON) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format_str, (const char*)str.GetBuffer())
#define YanLog(format_str, ...) if (JNI_LOG_ON) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format_str, __VA_ARGS__)
#endif

#endif // KLOGINFILE_H

// End of File

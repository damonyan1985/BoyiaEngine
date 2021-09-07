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

#if ENABLE(BOYIA_ANDROID)
#include <android/log.h>
#endif



#if ENABLE(BOYIA_KERNEL)
#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(log_str)
#define KDESLOG(log_str)
#define KSTRLOG(log_str)
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...)
#define KSTRFORMAT(format_str, str)

#elif ENABLE(BOYIA_ANDROID)
extern bool JNI_LOG_ON;

#define LOG_TAG "libboyia"
#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(...)   \
    if (JNI_LOG_ON) \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define KDESLOG(log_str)
#define KSTRLOG(log_str) \
    if (JNI_LOG_ON)      \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, (const char*)log_str.GetBuffer())
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...) \
    if (JNI_LOG_ON)                 \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format_str, __VA_ARGS__)
#define KSTRFORMAT(format_str, str) \
    if (JNI_LOG_ON)                 \
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, format_str, (const char*)str.GetBuffer())
#define BOYIA_LOG KFORMATLOG
#elif ENABLE(BOYIA_WINDOWS)
#include <stdio.h>
#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(log_str)
#define KDESLOG(log_str)
#define KSTRLOG(log_str)
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...) fprintf(stdout, format_str##"\n", __VA_ARGS__) 
#define KSTRFORMAT(format_str, str)
#define BOYIA_LOG KFORMATLOG
#elif ENABLE(BOYIA_IOS)

#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(log_str)
#define KDESLOG(log_str)
#define KSTRLOG(log_str)
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...)
#define KSTRFORMAT(format_str, str)
#define BOYIA_LOG KFORMATLOG
#else
#define INIT_KLOG(file_name, is_open)
#define SHUT_KLOG()
#define KLOG(log_str)
#define KDESLOG(log_str)
#define KSTRLOG(log_str)
#define KSTRLOG8(log_str)
#define KFORMATLOG(format_str, ...)
#define KSTRFORMAT(format_str, str)
#define BOYIA_LOG KFORMATLOG
#endif

#endif // KLOGINFILE_H

// End of File

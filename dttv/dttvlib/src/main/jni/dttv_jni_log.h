#ifndef DTTV_JNI_LOG_H
#define DTTV_JNI_LOG_H

#include <android/log.h>

#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#endif
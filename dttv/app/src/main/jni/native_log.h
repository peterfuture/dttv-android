/*
 * =====================================================================================
 *
 *    Filename   :  jni_log.h
 *    Description:
 *    Version    :  1.0
 *    Created    :  2016年04月11日 16时28分41秒
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s (), peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

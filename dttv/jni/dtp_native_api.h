#ifndef DTP_NATIVE_API_H
#define DTP_NATIVE_API_H

#include <jni.h>
#include <string.h>

int dtp_setDataSource(JNIEnv *env, jclass clazz, jstring url);
int dtp_prePare(JNIEnv *env, jclass clazz);
int dtp_prepareAsync(JNIEnv *env, jclass clazz);
int dtp_start(JNIEnv *env, jclass clazz);
int dtp_pause(JNIEnv *env, jclass clazz);
int dtp_seekTo(JNIEnv *env, jclass clazz, jint pos);
int dtp_stop(JNIEnv *env, jclass clazz);
int dtp_release(JNIEnv *env, jclass clazz);
int dtp_reset(JNIEnv *env, jclass clazz);
void dtp_releaseSurface(JNIEnv *env, jclass clazz);
int dtp_getVideoWidth(JNIEnv *env, jclass clazz);
int dtp_getVideoHeight(JNIEnv *env, jclass clazz);
int dtp_isPlaying(JNIEnv *env, jclass clazz);
//int dtp_setAdaptiveStream(JNIEnv *env, jclass clazz, boolean adaptive);
int dtp_getCurrentPosition(JNIEnv *env, jclass clazz);
int dtp_getDuration(JNIEnv *env, jclass clazz);
#endif

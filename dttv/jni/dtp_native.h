#ifndef DTP_NATIVE_H
#define DTP_NATIVE_H

#include <jni.h>
#include <string.h>

int native_playerStart(JNIEnv * env, jobject this, jstring url);
int native_playerPause(JNIEnv * env, jobject this);
int native_playerResume(JNIEnv * env, jobject this);
int native_playerSeekTo(JNIEnv * env, jobject this, jint pos);
int native_playerStop(JNIEnv * env, jobject this);

#endif

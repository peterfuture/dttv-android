#include <jni.h>
#include <string.h>
#include <android/log.h>

#define DEBUG_TAG "dttvJni"

//static void *handle;

void Java_com_example_simpleplayer_MainActivity_helloLog(JNIEnv * env, jobject this, jstring logThis)
{
    jboolean isCopy;
    const char * szLogThis = (*env)->GetStringUTFChars(env, logThis, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", szLogThis);
    (*env)->ReleaseStringUTFChars(env, logThis, szLogThis);
}

jint Java_com_example_simpleplayer_MainActivity_playerStart(JNIEnv * env, jobject this, jstring url)
{
    jboolean isCopy;
    const char * file_name = (*env)->GetStringUTFChars(env, url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive start cmd, file [%s]",file_name);
    return 0;
}

jint Java_com_example_simpleplayer_MainActivity_playerPause(JNIEnv * env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive pause cmd");
    return 0;
}
jint Java_com_example_simpleplayer_MainActivity_playerResume(JNIEnv * env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive resume cmd");
    return 0;
}

jint Java_com_example_simpleplayer_MainActivity_playerSeek(JNIEnv * env, jobject this, jint pos)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive seek cmd, seek to %d ",pos);
    return 0;
}

jint Java_com_example_simpleplayer_MainActivity_playerStop(JNIEnv * env, jobject this)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive pause cmd");
    return 0;
}

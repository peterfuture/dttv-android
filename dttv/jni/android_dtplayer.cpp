#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif
#define LOG_TAG "DTPLAYER-JNI"

#include "android_runtime/AndroidRuntime.h"  
#include <jni.h>
#include <android/log.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "dtp_native_api.h"

// ------------------------------------------------------------

using namespace android;

// ------------------------------------------------------------

// ----------------------------------------------------------------------------

static const char * const kClassName = "dttv/app/DtPlayer";
static DTPlayer *dtPlayer = NULL; // player handle

//================================================

// Notify to Java
int Notify(int status)
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    jclass mClass = env->FindClass("dttv/app/DtPlayer");
    jmethodID notify_cb = env->GetStaticMethodID(mClass, "updateState", "(I)V");
    if(!notify_cb || !mClass)
    {
        return -1;
    }
    env->CallStaticVoidMethod(mClass,notify_cb,status);
    return 0;
}

//================================================

int dtp_nativeSetup(JNIEnv *env, jobject obj)
{
    if(dtPlayer)
    {
        return -1;
    }
    dtPlayer = new DTPlayer;
    
    return 0;
}

int dtp_setDataSource(JNIEnv *env, jobject obj, jstring url)
{
    int ret = 0;
    jboolean isCopy;
    const char * file_name = env->GetStringUTFChars(url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setDataSource, path: [%s] size:%d ",file_name,strlen(file_name));

    if(!dtPlayer)
    {
        dtp_nativeSetup(env,obj);
    }
    else
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setDataSource, failed, already done ");
        return 0;
    }

    ret = dtPlayer->setDataSource(file_name);
    if(ret < 0)
    {
        return -1;
    }
    return 0;
}

int dtp_prePare(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    dtPlayer->prePare();
    Notify(MEDIA_PREPARED);
    return 0;
}

int dtp_prepareAsync(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    dtPlayer->prePareAsync();
    Notify(MEDIA_PREPARED);
    return 0;
}

int dtp_start(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->start();
}

int dtp_pause(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->pause();
}

int dtp_seekTo(JNIEnv *env, jobject obj, jint pos)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->seekTo(pos);
}

int dtp_stop(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    dtPlayer->stop();
    return 0;
}

int dtp_release(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    dtPlayer->release();
}

int dtp_reset(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->reset();
}

void dtp_releaseSurface(JNIEnv *env, jobject obj)
{

}

int dtp_getVideoWidth(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->getVideoWidth();
}

int dtp_getVideoHeight(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    return dtPlayer->getVideoHeight();
}

int dtp_isPlaying(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return 0;
    return dtPlayer->isPlaying();
}

int dtp_getCurrentPosition(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return 0;
    return dtPlayer->getCurrentPosition();
}

int dtp_getDuration(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return 0;
    return dtPlayer->getDuration();
}

static JNINativeMethod g_Methods[] = {
    //New API
    {"native_setDataSource",      "(Ljava/lang/String;)I",    (void*) dtp_setDataSource},
    {"native_prePare",            "()I",                      (void*) dtp_prePare},
    {"native_prePareAsync",       "()I",                      (void*) dtp_prepareAsync},
    {"native_start",              "()I",                      (void*) dtp_start},
    {"native_pause",              "()I",                      (void*) dtp_pause},
    {"native_seekTo",             "(I)I",                     (void*) dtp_seekTo},
    {"native_stop",               "()I",                      (void*) dtp_stop},
    {"native_release",            "()I",                      (void*) dtp_release},
    {"native_reset",              "()I",                      (void*) dtp_reset},
    {"native_getVideoWidth",      "()I",                      (void*) dtp_getVideoWidth},
    {"native_getVideoHeight",     "()I",                      (void*) dtp_getVideoHeight},
    {"native_isPlaying",          "()I",                      (void*) dtp_isPlaying},
    {"native_getCurrentPosition", "()I",                      (void*) dtp_getCurrentPosition},
    {"native_getDuration",        "()I",                      (void*) dtp_getDuration},
};

static int register_android_dtplayer(JNIEnv *env)
{
    jclass clazz;
    clazz = env->FindClass(kClassName);
	if (clazz == NULL) {
		fprintf(stderr, "Native registration unable to find class '%s'\n",kClassName);
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, g_Methods, NELEM(g_Methods)) < 0) {
		fprintf(stderr, "RegisterNatives failed for '%s'\n", kClassName);
		return JNI_FALSE;
	}

    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        //ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_android_dtplayer(env) < 0) {
        //ALOGE("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

bail:
    return result;
}



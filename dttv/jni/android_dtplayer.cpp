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

#include "android_dtplayer.h"
#include "dtp_native_api.h"

// ------------------------------------------------------------

using namespace android;

// ------------------------------------------------------------

// ----------------------------------------------------------------------------

static const char * const kClassName = "dttv/app/DtPlayer";
static DTPlayer *dtPlayer = NULL; // player handle

//================================================

static JavaVM *gvm = NULL;
static jclass mClass = NULL;
static jmethodID notify_cb = NULL;


// Notify to Java
int Notify(int status)
{
    JNIEnv *env = NULL;
    int isAttached = 0; 

    if(gvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm getenv failed use AttachCurrentThread \n ");
        if(gvm->AttachCurrentThread(&env, NULL) != JNI_OK)
        {
            __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm AttachCurrentThread failed \n ");
            return -1;
        }
        isAttached = 1;
    }
    else
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm getenv ok \n ");
#if 0 
    mClass = env->FindClass("dttv/app/DtPlayer");
    if(!mClass)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "jvm getenv failed \n ");
        goto END;
    }
    else
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "env find class ok \n ");

    notify_cb = env->GetStaticMethodID(mClass, "updateState", "(I)V");
#endif
    
    if(!notify_cb || !mClass)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "updateState can not found ");
        goto END;
    }
    env->CallStaticVoidMethod(mClass,notify_cb,status);
END:
    if(isAttached)
        gvm->DetachCurrentThread();  

    return 0;
}

dtpListenner::dtpListenner(JNIEnv *env, jobject thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    mClass = (jclass)env->NewGlobalRef(clazz);
    mObj = env->NewGlobalRef(thiz);
//    notify_cb = env->GetMethodID(mClass, "updateState", "(I)V");
}

dtpListenner::~dtpListenner()
{
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->DeleteGlobalRef(mObj);
    env->DeleteGlobalRef(mClass);
}

int dtpListenner::notify(int status)
{
#if 0
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    //jmethodID notify_cb = env->GetStaticMethodID(mClass, "updateState", "(I)V");
    if(!notify_cb || !mClass)
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "updateState can not found ");
        return -1;
    }
    //env->CallSTATICVoidMethod(mClass,notify_cb,status);
    env->CallVoidMethod(mObj,notify_cb,status);
#endif
    return 0;
}

//================================================

int dtp_setDataSource(JNIEnv *env, jobject obj, jstring url)
{
    int ret = 0;
    jboolean isCopy;
    const char * file_name = env->GetStringUTFChars(url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setDataSource, path: [%s] size:%d ",file_name,strlen(file_name));

    jclass clazz = env->GetObjectClass(obj);
    mClass = (jclass)env->NewGlobalRef(clazz);
    notify_cb = env->GetStaticMethodID(mClass, "updateState", "(I)V");

    if(!dtPlayer)
    {
        dtPlayer = new DTPlayer;
        dtpListenner *listenner = new dtpListenner(env, obj);
        dtPlayer->setListenner(listenner);
    }
    else
    {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "setDataSource, failed, already done ");
        return -1;
    }

    return dtPlayer->setDataSource(file_name);
}

int dtp_prePare(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    dtPlayer->prePare();
    return 0;
}

int dtp_prepareAsync(JNIEnv *env, jobject obj)
{
    if(!dtPlayer)
        return -1;
    dtPlayer->prePareAsync();
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
    int ret = 0;
    if(!dtPlayer)
        return -1;

    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "native stop enter \n ");
    ret = dtPlayer->stop();
    {
        if(ret == -1)
            return -1;
    }
    while(dtPlayer->isQuitOK() == 0)
    {
        usleep(10000);
    }
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "native stop exit \n ");
    delete dtPlayer;
    dtPlayer = NULL;
    return 0;
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
        return -1;
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
    gvm = vm;

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



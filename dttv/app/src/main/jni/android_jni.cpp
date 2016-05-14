#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "native_log.h"
#include "android_dtplayer.h"
#include "android_jni.h"
#include "android_opengl.h"

#include "native_log.h"
#define TAG "DTPLAYER-JNI"
#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif

using namespace android;

// ------------------------------------------------------------

//------------------------------------------------------------
//jni code
struct fields_t {
    jfieldID    context;
    jfieldID    surface_texture;

    jmethodID   post_event;

    jmethodID   proxyConfigGetHost;
    jmethodID   proxyConfigGetPort;
    jmethodID   proxyConfigGetExclusionList;
};
static fields_t fields;
dt_lock_t sLock;
// ----------------------------------------------------------------------------

static JavaVM *gvm = NULL;
static const char * const kClassName = "dttv/app/DtPlayer";

//================================================

dtpListenner::dtpListenner(JNIEnv *env, jobject thiz, jobject weak_thiz)
{
    jclass clazz = env->GetObjectClass(thiz);
    if(clazz == NULL)
    {
    	LOGV( "can not find DtPlayer \n ");
        return;
    }
    mClass = (jclass)env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
}

dtpListenner::~dtpListenner()
{
//    JNIEnv *env = AndroidRuntime::getJNIEnv();
//    env->DeleteGlobalRef(mObject);
//    env->DeleteGlobalRef(mClass);
}

int dtpListenner::notify(int msg, int ext1, int ext2)
{
    JNIEnv *env = NULL;
    int isAttached = 0; 
    if(gvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
    	LOGV( "jvm getenv failed use AttachCurrentThread \n ");
        if(gvm->AttachCurrentThread(&env, NULL) != JNI_OK)
        {
        	LOGV( "jvm AttachCurrentThread failed \n ");
            return -1;
        }
        isAttached = 1;
    }
    
    if(!fields.post_event || !mClass)
    {
    	LOGV( "updateState can not found ");
        goto END;
    }
    
    env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, NULL);
    LOGV( "NOtify with listener \n ");
END:
    if(isAttached)
        gvm->DetachCurrentThread();  

#if 0
    LOGV( "Enter notify \n ");
    JNIEnv *env = AndroidRuntime::getJNIEnv();
    env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, NULL);

    if(env->ExceptionCheck())
    {
    	LOGV( "An Exception occured while notifying an event \n ");
        env->ExceptionClear();
    }
#endif
    return 0;
}

static DTPlayer * setMediaPlayer(JNIEnv *env, jobject thiz, DTPlayer *player)
{
    dt_lock(&sLock);
    DTPlayer *old = (DTPlayer *)env->GetIntField(thiz, fields.context);
    //env->SetIntField(thiz, fields.context, (u_long)player);
    env->SetLongField(thiz, fields.context, (u_long)player);
    dt_unlock(&sLock);
    return old;
}

static DTPlayer *getMediaPlayer(JNIEnv *env, jobject thiz)
{
    dt_lock(&sLock);
    DTPlayer *dtp = (DTPlayer *)env->GetIntField(thiz, fields.context);
    dt_unlock(&sLock);
    return dtp;
}

//================================================

static void android_dtplayer_native_init(JNIEnv *env)
{
    jclass clazz;

    clazz = env->FindClass(kClassName);
    if (clazz == NULL) {
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "I");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        return;
    }

#if 0
    fields.surface_texture = env->GetFieldID(clazz, "mNativeSurfaceTexture", "I");
    if (fields.surface_texture == NULL) {
        return;
    }
#endif

    gles2_setup();

}

static int android_dtplayer_native_setup(JNIEnv *env, jobject obj, jobject weak_thiz)
{
    DTPlayer *mp = new DTPlayer();
    if(mp == NULL)
    {
        //jniThrowExcption(env, "java/lang/RuntimeException", "Out of memory");
        return -1;
    }
    dtpListenner *listenner = new dtpListenner(env, obj, weak_thiz);
    mp->setListenner(listenner);
    setMediaPlayer(env, obj, mp);
    return 0;
}

static int android_dtplayer_native_hw_enable(JNIEnv *env, jobject thiz, jint enable)
{
	LOGV( "Enter hw codec enable set, enable:%d ", enable);

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
    {
    	LOGV(  "set hw enable failed, mp == null ");
        return -1;
    }
    
    return mp->setHWEnable(enable);
}

static int android_dtplayer_native_release(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = setMediaPlayer(env, thiz, 0);
    if(mp)
    {
        delete mp;
    }
    return 0;
}

int android_dtplayer_native_setDataSource(JNIEnv *env, jobject thiz, jstring url)
{
    int ret = 0;
    jboolean isCopy;
    const char * file_name = env->GetStringUTFChars(url, &isCopy);
    LOGV( "Enter setDataSource, path: [%s] size:%d ",file_name,strlen(file_name));

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
    {
    	LOGV( "setDataSource, failed, mp == null ");
        return -1;
    }
    
    ret = mp->setDataSource(file_name);
    if(ret < 0)
    {
    	LOGV( "setDataSource, failed, ret:%d ", ret);
    }
    return ret;
}

int android_dtplayer_native_prePare(JNIEnv *env, jobject thiz)
{
	LOGV( "Enter prePare");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;

    mp->prePare();
    return 0;
}

int android_dtplayer_native_prepareAsync(JNIEnv *env, jobject thiz)
{
	LOGV( "Enter prePareAsync");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    mp->prePareAsync();
    return 0;
}

int android_dtplayer_native_start(JNIEnv *env, jobject thiz)
{
	LOGV( "Enter start");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->start();
}

int android_dtplayer_native_pause(JNIEnv *env, jobject thiz)
{
	LOGV( "Enter pause");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->pause();
}

int android_dtplayer_native_seekTo(JNIEnv *env, jobject thiz, jint pos)
{
	LOGV( "Enter seekTo pos:%d s",pos);
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->seekTo(pos);
}

int android_dtplayer_native_stop(JNIEnv *env, jobject thiz)
{
	LOGV( "Enter stop ");
    int ret = 0;
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    LOGV( "native stop enter \n ");
    ret = mp->stop();
    if(ret == -1)
        return -1;
    while(mp->isQuitOK() == 0)
    {
        usleep(10000);
    }

    LOGV( "native stop exit \n ");
    return 0;
}

int android_dtplayer_native_reset(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->reset();
}

void android_dtplayer_native_releaseSurface(JNIEnv *env, jobject thiz)
{

}

int android_dtplayer_native_setVideoSize(JNIEnv *env, jobject obj, int w, int h)
{
    return 0;
}

int android_dtplayer_native_setVideoMode(JNIEnv *env, jobject obj, int mode)
{
    return 0;
}

int android_dtplayer_native_getVideoWidth(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->getVideoWidth();
}

int android_dtplayer_native_getVideoHeight(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->getVideoHeight();
}

int android_dtplayer_native_isPlaying(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->isPlaying();
}

int android_dtplayer_native_getCurrentPosition(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->getCurrentPosition();
}

int android_dtplayer_native_getDuration(JNIEnv *env, jobject thiz)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
        return -1;
    return mp->getDuration();
}

int android_dtplayer_native_onSurfaceCreated(JNIEnv *env, jobject thiz)
{
    gles2_init();
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp)
        gles2_reg_player(mp);
    else
    	LOGV( "onSurfaceCreated register player failed mp null\n");

    return 0;
}

int android_dtplayer_native_onSurfaceChanged(JNIEnv *env, jobject obj, int w, int h)
{

    gles2_surface_changed(w, h);
    LOGV( "onSurfaceChanged, w:%d h:%d \n",w,h);
    android_dtplayer_native_setVideoSize(env, obj, w, h);
    return 0;
}

int android_dtplayer_native_onDrawFrame(JNIEnv *env, jobject thiz)
{
    gles2_draw_frame();
    return 0;
}

//-----------------------------------------------------

//-----------------------------------------------------
// Audio Effect Interface
static int android_dtplayer_native_setAudioEffect(JNIEnv *env, jobject thiz , jint id)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if(mp == NULL)
    {
    	LOGV( "setDataSource, failed, mp == null ");
        return -1;
    }
    mp->setAudioEffect(id); 
    return 0;
}

//-----------------------------------------------------

static JNINativeMethod g_Methods[] = {
    //New API
    {"native_init",               "()V",                      (void*) android_dtplayer_native_init},
    {"native_setup",              "(Ljava/lang/Object;)I",    (void*) android_dtplayer_native_setup},
    {"native_hw_enable",          "(I)I",                     (void*) android_dtplayer_native_hw_enable},
    {"native_release",            "()I",                      (void*) android_dtplayer_native_release},
    {"native_setDataSource",      "(Ljava/lang/String;)I",    (void*) android_dtplayer_native_setDataSource},
    {"native_prePare",            "()I",                      (void*) android_dtplayer_native_prePare},
    {"native_prePareAsync",       "()I",                      (void*) android_dtplayer_native_prepareAsync},
    {"native_start",              "()I",                      (void*) android_dtplayer_native_start},
    {"native_pause",              "()I",                      (void*) android_dtplayer_native_pause},
    {"native_seekTo",             "(I)I",                     (void*) android_dtplayer_native_seekTo},
    {"native_stop",               "()I",                      (void*) android_dtplayer_native_stop},
    {"native_reset",              "()I",                      (void*) android_dtplayer_native_reset},
    {"native_setVideoMode",       "(I)I",                     (void*) android_dtplayer_native_setVideoMode},
    {"native_setVideoSize",       "(II)I",                    (void*) android_dtplayer_native_setVideoSize},
    {"native_getVideoWidth",      "()I",                      (void*) android_dtplayer_native_getVideoWidth},
    {"native_getVideoHeight",     "()I",                      (void*) android_dtplayer_native_getVideoHeight},
    {"native_isPlaying",          "()I",                      (void*) android_dtplayer_native_isPlaying},
    {"native_getCurrentPosition", "()I",                      (void*) android_dtplayer_native_getCurrentPosition},
    {"native_getDuration",        "()I",                      (void*) android_dtplayer_native_getDuration},
    
    {"native_onSurfaceCreated",   "()I",                      (void*) android_dtplayer_native_onSurfaceCreated},
    {"native_onSurfaceChanged",   "(II)I",                    (void*) android_dtplayer_native_onSurfaceChanged},
    {"native_onDrawFrame",        "()I",                      (void*) android_dtplayer_native_onDrawFrame},
    
    {"native_setAudioEffect",     "(I)I",                     (void*) android_dtplayer_native_setAudioEffect},
};

static int register_android_dtplayer(JNIEnv *env)
{
#if 1
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
#else
    return AndroidRuntime::registerNativeMethods(env, kClassName, g_Methods, NELEM(g_Methods));
#endif

}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;
    gvm = vm;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGV("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_android_dtplayer(env) < 0) {
    	LOGV("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;
    dt_lock_init(&sLock, NULL);
bail:
    return result;
}



#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include "native_log.h"
#include "android_dtplayer.h"
#include "android_jni.h"
#include "gl_yuv.h"
#include "jni_utils.h"

#include "native_log.h"



#define TAG "DTTV-JNI"

#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
#endif

using namespace android;

struct fields_t {
    jfieldID context;
    jfieldID surface_texture;

    jmethodID post_event;

    jmethodID proxyConfigGetHost;
    jmethodID proxyConfigGetPort;
    jmethodID proxyConfigGetExclusionList;
};
static fields_t fields;
lock_t mutex;
static JavaVM *gvm = NULL;
static const char *const kClassName = "dttv/app/MediaPlayer";

enum {
    KEY_PARAMETER_USEHWCODEC = 0x0,
    KEY_PARAMETER_MAX
};

extern "C" int av_jni_set_java_vm(void *vm, void *log_ctx);

dtpListenner::dtpListenner(JNIEnv *env, jobject thiz, jobject weak_thiz) {
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        LOGV("can not find DtPlayer \n ");
        return;
    }
    mClass = (jclass) env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
    LOGV("dttv listenner construct ok");
}

dtpListenner::~dtpListenner() {
    //    JNIEnv *env = AndroidRuntime::getJNIEnv();
    //    env->DeleteGlobalRef(mObject);
    //    env->DeleteGlobalRef(mClass);
}

int dtpListenner::notify(int msg, int ext1, int ext2) {
    JNIEnv *env = NULL;
    int isAttached = 0;
    if (gvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGV("jvm getenv failed use AttachCurrentThread \n ");
        if (gvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            LOGV("jvm AttachCurrentThread failed \n ");
            return -1;
        }
        isAttached = 1;
    }

    if (!fields.post_event || !mClass) {
        LOGV("updateState can not found ");
        goto END;
    }

    env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, NULL);
    LOGV("NOtify with listener \n ");
    END:
    if (isAttached) {
        gvm->DetachCurrentThread();
    }

    return 0;
}

static DTPlayer *setMediaPlayer(JNIEnv *env, jobject thiz, DTPlayer *player) {
    lock(&mutex);
    DTPlayer *old = (DTPlayer *) env->GetLongField(thiz, fields.context);
    env->SetLongField(thiz, fields.context, (jlong) player);
    unlock(&mutex);
    return old;
}

static DTPlayer *getMediaPlayer(JNIEnv *env, jobject thiz) {
    lock(&mutex);
    DTPlayer *dtp = (DTPlayer *) env->GetLongField(thiz, fields.context);
    unlock(&mutex);
    return dtp;
}


static void jni_dttv_init(JNIEnv *env) {
    jclass clazz;

    clazz = env->FindClass(kClassName);
    if (clazz == NULL) {
        return;
    }

    fields.context = env->GetFieldID(clazz, "mNativeContext", "J");
    if (fields.context == NULL) {
        return;
    }

    fields.post_event = env->GetStaticMethodID(clazz, "postEventFromNative",
                                               "(Ljava/lang/Object;IIILjava/lang/Object;)V");
    if (fields.post_event == NULL) {
        return;
    }

}

static int jni_dttv_setup(JNIEnv *env, jobject obj, jobject weak_thiz) {

    dtpListenner *listenner = new dtpListenner(env, obj, weak_thiz);

    DTPlayer *mp = new DTPlayer(listenner);
    if (mp == NULL) {
        //jniThrowExcption(env, "java/lang/RuntimeException", "Out of memory");
        LOGV("Error: dtplayer create failed");
        return -1;
    }
    //mp->setListenner(listenner);
    setMediaPlayer(env, obj, mp);
    LOGV("native dttv setup ok");
    return 0;
}

static int jni_dttv_hw_enable(JNIEnv *env, jobject thiz, jint enable) {
    LOGV("Enter hw codec enable set, enable:%d ", enable);

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set hw enable failed, mp == null ");
        return -1;
    }

    return mp->setHWEnable(enable);
}

static int jni_dttv_release(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = setMediaPlayer(env, thiz, 0);
    if (mp) {
        delete mp;
    }
    return 0;
}

void jni_dttv_set_datasource(JNIEnv *env, jobject thiz, jstring url) {
    int ret = 0;
    jboolean isCopy;
    const char *file_name = env->GetStringUTFChars(url, &isCopy);
    LOGV("Enter setDataSource, path: [%s] size:%d ", file_name, strlen(file_name));

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("setDataSource, failed, mp == null ");
        return;
    }

    ret = mp->setDataSource(file_name);
    if (ret < 0) {
        LOGV("setDataSource, failed, ret:%d ", ret);
    }
    return;
}

int jni_dttv_prePare(JNIEnv *env, jobject thiz) {
    LOGV("Enter prePare");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }

    mp->prePare();
    return 0;
}

int jni_dttv_prepareAsync(JNIEnv *env, jobject thiz) {
    LOGV("Enter prePareAsync");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    mp->prePareAsync();
    return 0;
}

int jni_dttv_start(JNIEnv *env, jobject thiz) {
    LOGV("Enter start");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->start();
}

int jni_dttv_pause(JNIEnv *env, jobject thiz) {
    LOGV("Enter pause");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->pause();
}

int jni_dttv_seekTo(JNIEnv *env, jobject thiz, jint pos) {
    LOGV("Enter seekTo pos:%d s", pos);
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->seekTo(pos);
}

int jni_dttv_stop(JNIEnv *env, jobject thiz) {
    LOGV("Enter stop ");
    int ret = 0;
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    LOGV("native stop enter \n ");
    ret = mp->stop();
    if (ret == -1) {
        return -1;
    }
    while (mp->isQuitOK() == 0) {
        usleep(10000);
    }

    LOGV("native stop exit \n ");
    return 0;
}

int jni_dttv_reset(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->reset();
}

void jni_dttv_releaseSurface(JNIEnv *env, jobject thiz) {

}

int jni_dttv_setVideoSize(JNIEnv *env, jobject obj, int w, int h) {
    return 0;
}

int jni_dttv_get_video_width(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->getVideoWidth();
}

int jni_dttv_get_video_height(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->getVideoHeight();
}

int jni_dttv_is_playing(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->isPlaying();
}

int jni_dttv_get_current_position(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->getCurrentPosition();
}

int jni_dttv_get_duration(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }
    return mp->getDuration();
}

int jni_dttv_set_parameter(JNIEnv *env, jobject thiz, int cmd, jlong arg1, jlong arg2) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return -1;
    }

    switch (cmd) {
        case KEY_PARAMETER_USEHWCODEC:
            mp->setHWEnable(arg1);
            break;
        default:
            break;
    }
    return 0;
}

int jni_gl_surface_create(JNIEnv *env, jobject thiz) {
    yuv_dttv_init();
    yuv_reg_player((void *) getMediaPlayer(env, thiz));
    return 0;
}

int jni_gl_surface_change(JNIEnv *env, jobject obj, int w, int h) {
    yuv_setupGraphics(w, h);
    LOGV("on surface changed, w:%d h:%d \n", w, h);
    return 0;
}


int jni_gl_draw_frame(JNIEnv *env, jobject thiz) {
    yuv_renderFrame();
    return 0;
}

static int jni_dttv_set_audio_effect(JNIEnv *env, jobject thiz, jint id) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("setDataSource, failed, mp == null ");
        return -1;
    }
    mp->setAudioEffect(id);
    return 0;
}

static JNINativeMethod g_Methods[] = {
        {"native_init",                 "()V",                   (void *) jni_dttv_init},
        {"native_setup",                "(Ljava/lang/Object;)I", (void *) jni_dttv_setup},
        {"native_release",              "()I",                   (void *) jni_dttv_release},
        {"native_set_datasource",       "(Ljava/lang/String;)V", (void *) jni_dttv_set_datasource},
        {"native_prepare",              "()I",                   (void *) jni_dttv_prePare},
        {"native_prepare_async",        "()I",                   (void *) jni_dttv_prepareAsync},
        {"native_start",                "()I",                   (void *) jni_dttv_start},
        {"native_pause",                "()I",                   (void *) jni_dttv_pause},
        {"native_seekTo",               "(I)I",                  (void *) jni_dttv_seekTo},
        {"native_stop",                 "()I",                   (void *) jni_dttv_stop},
        {"native_reset",                "()I",                   (void *) jni_dttv_reset},
        {"native_get_video_width",      "()I",                   (void *) jni_dttv_get_video_width},
        {"native_get_video_height",     "()I",                   (void *) jni_dttv_get_video_height},
        {"native_is_playing",           "()I",                   (void *) jni_dttv_is_playing},
        {"native_get_current_position", "()I",                   (void *) jni_dttv_get_current_position},
        {"native_get_duration",         "()I",                   (void *) jni_dttv_get_duration},

        {"native_set_parameter",        "(IJJ)I",                (void *) jni_dttv_set_parameter},

        {"native_surface_create",       "()I",                   (void *) jni_gl_surface_create},
        {"native_surface_change",       "(II)I",                 (void *) jni_gl_surface_change},
        {"native_draw_frame",           "()I",                   (void *) jni_gl_draw_frame},

        {"native_set_audio_effect",     "(I)I",                  (void *) jni_dttv_set_audio_effect},
};

static int register_natives(JNIEnv *env) {
    jclass clazz;
    clazz = env->FindClass(kClassName);
    if (clazz == NULL) {
        fprintf(stderr, "Native registration unable to find class '%s'\n", kClassName);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, g_Methods, NELEM(g_Methods)) < 0) {
        fprintf(stderr, "RegisterNatives failed for '%s'\n", kClassName);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jint result = -1;
    gvm = vm;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGV("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_natives(env) < 0) {
        LOGV("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;
    lock_init(&mutex, NULL);

    av_jni_set_java_vm(vm, reserved);
    bail:
    return result;
}



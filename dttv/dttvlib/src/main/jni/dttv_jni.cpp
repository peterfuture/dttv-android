#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include <plugin/gl_yuv.h>
#include "dttv_jni_dtp.h"

#define TAG "DTTV-JNI"

using namespace android;

enum {
    KEY_PARAMETER_USEHWCODEC = 0x0,
    KEY_PARAMETER_MAX
};

struct fields_t {
    jfieldID context;
    jfieldID surface_texture;
    jmethodID post_event;
};

static fields_t fields;
static lock_t mutex;
static JavaVM *gvm = NULL;

// ------------------------------------------------------
// CallBack Listenner Impl

dttvListenner::dttvListenner(JNIEnv *env, jobject thiz, jobject weak_thiz) {
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == NULL) {
        LOGV("can not find DtPlayer \n ");
        return;
    }
    mClass = (jclass) env->NewGlobalRef(clazz);
    mObject = env->NewGlobalRef(weak_thiz);
    LOGV("dttv listenner construct.");
}

dttvListenner::~dttvListenner() {
    int isAttached = 0;
    JNIEnv *env = NULL;
    if (gvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGV("jvm getenv failed use AttachCurrentThread \n ");
        if (gvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            LOGV("jvm AttachCurrentThread failed \n ");
            return;
        }
        isAttached = 1;
    }

    if (isAttached < 0)
        return;

    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
    LOGV("dttv listenner destruct.");
}

int dttvListenner::notify(int msg, int ext1, int ext2) {
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

// ------------------------------------------------------

static DTPlayer *setMediaPlayer(JNIEnv *env, jobject thiz, DTPlayer *player) {
    DTPlayer *old = NULL;
    lock(&mutex);
    old = (DTPlayer *) env->GetLongField(thiz, fields.context);
    env->SetLongField(thiz, fields.context, (jlong) player);
    unlock(&mutex);
    return old;
}

static DTPlayer *getMediaPlayer(JNIEnv *env, jobject thiz) {
    DTPlayer *dtp = NULL;
    lock(&mutex);
    dtp = (DTPlayer *) env->GetLongField(thiz, fields.context);
    unlock(&mutex);
    return dtp;
}

static void jni_dttv_init(JNIEnv *env) {
    jclass clazz;

    clazz = env->FindClass("app/dttv/dttvlib/MediaPlayer");
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

    dttvListenner *listenner = new dttvListenner(env, obj, weak_thiz);
    DTPlayer *mp = new DTPlayer(listenner);
    if (mp == NULL) {
        //jniThrowExcption(env, "java/lang/RuntimeException", "Out of memory");
        LOGV("Error: dtplayer create failed.");
        delete listenner;
        return -1;
    }
    //mp->setListenner(listenner);
    setMediaPlayer(env, obj, mp);
    LOGV("native dttv setup ok");
    return 0;
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

int jni_dttv_prepare(JNIEnv *env, jobject thiz) {
    LOGV("Enter prePare");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return -1;
    }

    mp->prePare();
    return 0;
}

int jni_dttv_prepareasync(JNIEnv *env, jobject thiz) {
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
    mp->stop();
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

static void jni_dttv_set_video_surface(JNIEnv *env, jobject thiz, jobject jsurface)
{
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return;
    }
    ANativeWindow *window = (ANativeWindow *)ANativeWindow_fromSurface(env, jsurface);
    LOGV("NativeWindow Address:%p \n", window);
    mp->setNativeWindow(window);
}

int jni_gl_surface_create(JNIEnv *env, jobject thiz) {

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return 0;
    }
    yuv_dttv_init();
    yuv_reg_player((void *) getMediaPlayer(env, thiz));

    return 0;
}

int jni_gl_surface_change(JNIEnv *env, jobject thiz, int w, int h) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return 0;
    }
    yuv_setupGraphics(w, h);
    mp->setGLSurfaceView();
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
        {"native_prepare",              "()I",                   (void *) jni_dttv_prepare},
        {"native_prepare_async",        "()I",                   (void *) jni_dttv_prepareasync},
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

        {"native_set_video_surface",    "(Landroid/view/Surface;)V", (void *)jni_dttv_set_video_surface},

        {"native_surface_create",       "()I",                   (void *) jni_gl_surface_create},
        {"native_surface_change",       "(II)I",                 (void *) jni_gl_surface_change},
        {"native_draw_frame",           "()I",                   (void *) jni_gl_draw_frame},

        {"native_set_audio_effect",     "(I)I",                  (void *) jni_dttv_set_audio_effect},
};

#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
extern "C" int av_jni_set_java_vm(void *vm, void *log_ctx);

static int register_natives(JNIEnv *env) {
    jclass clazz;
    clazz = env->FindClass("app/dttv/dttvlib/MediaPlayer");
    if (clazz == NULL) {
        LOGV("Native registration unable to find MediaPlayer class.\n");
        return JNI_FALSE;
    }


    if (env->RegisterNatives(clazz, g_Methods, NELEM(g_Methods)) < 0) {
        LOGV("RegisterNatives failed.\n");
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

    result = JNI_VERSION_1_4;
    lock_init(&mutex, NULL);
    av_jni_set_java_vm(vm, reserved);
    bail:
    return result;
}
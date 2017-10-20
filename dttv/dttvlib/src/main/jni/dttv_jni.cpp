#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#include <gl_render.h>
#include <dtp_state.h>
#include "dttv_jni_dtp.h"
#include "dttv_jni_surface.h"
#include <dttv_jni_cmd.h>

#include <dttv_jni.h>
#include <pthread.h>

#define TAG "DTTV-JNI"

using namespace android;

struct fields_t {
    jfieldID context;
    jfieldID surface_texture;
    jmethodID post_event;
};

static fields_t fields;
static lock_t mutex;

// JNI
static JavaVM *java_vm;
static pthread_key_t current_env;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void jni_detach_env(void *data)
{
    if (java_vm) {
        java_vm->DetachCurrentThread();
    }
}

static void jni_create_pthread_key(void)
{
    pthread_key_create(&current_env, jni_detach_env);
}

JNIEnv *ff_jni_get_env()
{
    int ret = 0;
    JNIEnv *env = NULL;

    pthread_mutex_lock(&lock);
    if (!java_vm) {
        goto done;
    }

    pthread_once(&once, jni_create_pthread_key);

    if ((env = (JNIEnv *)pthread_getspecific(current_env)) != NULL) {
        goto done;
    }

    ret = java_vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    switch(ret) {
        case JNI_EDETACHED:
            if (java_vm->AttachCurrentThread(&env, NULL) != 0) {
                env = NULL;
            } else {
                pthread_setspecific(current_env, env);
            }
            break;
        case JNI_OK:
            break;
        case JNI_EVERSION:
            break;
        default:
            break;
    }

    done:
    pthread_mutex_unlock(&lock);
    return env;
}

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
    LOGV("dttv listenner destruct. enter");
    JNIEnv *env = ff_jni_get_env();
    env->DeleteGlobalRef(mObject);
    env->DeleteGlobalRef(mClass);
    LOGV("dttv listenner destruct.");
}

int dttvListenner::notify(int msg, int ext1, int ext2) {
    JNIEnv *env = ff_jni_get_env();
    if (!fields.post_event || !mClass) {
        LOGV("updateState can not found ");
        return 0;
    }
    env->CallStaticVoidMethod(mClass, fields.post_event, mObject, msg, ext1, ext2, NULL);
    if(env->ExceptionCheck()) {
        LOGV("An exception occurered while nofifying an event.\n ");
    }
    LOGV("NOtify with listener \n ");
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

    DTPlayer *mp = new DTPlayer();
    if (mp == NULL) {
        LOGV("Error: dtplayer create failed.");
        return -1;
    }
    dttvListenner *listenner = new dttvListenner(env, obj, weak_thiz);
    mp->setListenner(listenner);
    setMediaPlayer(env, obj, mp);
    LOGV("native dttv setup ok");
    return 0;
}

static int jni_dttv_release(JNIEnv *env, jobject thiz) {
    DTPlayer *mp = setMediaPlayer(env, thiz, 0);
    return 0;
    if (mp) {
        dttvListenner *listenner = mp->getListenner();
        mp->setListenner(NULL);
        if(listenner) {
            delete listenner;
        }
        delete mp;
    }
    return 0;
}

static void jni_dttv_native_finalize(JNIEnv *env, jobject thiz) {
    LOGV("native_finalize");
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp != NULL) {
        LOGV("MediaPlayer finalized without being released");
    }
    jni_dttv_release(env, thiz);
}

static void jni_dttv_setDataSourceAndHeaders(
        JNIEnv *env, jobject thiz, jstring path,
        jobjectArray keys, jobjectArray values) {

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return;
    }

    if (path == NULL) {
        return;
    }

    const char *tmp = env->GetStringUTFChars(path, NULL);
    if (tmp == NULL) {  // Out of memory
        return;
    }
    LOGV("setDataSource: path %s", tmp);
    mp->setDataSource(tmp);
#if 0
    String8 pathStr(tmp);
    env->ReleaseStringUTFChars(path, tmp);
    tmp = NULL;

    // We build a KeyedVector out of the key and val arrays
    KeyedVector<String8, String8> headersVector;
    if (!ConvertKeyValueArraysToKeyedVector(
            env, keys, values, &headersVector)) {
        return;
    }

    status_t opStatus =
            mp->setDataSource(
                    pathStr,
                    headersVector.size() > 0? &headersVector : NULL);
#endif
}

static void jni_dttv_setDataSourceFD(JNIEnv *env, jobject thiz, jobject fileDescriptor,
                                     jlong offset, jlong length) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        return;
    }

    if (fileDescriptor == NULL) {
        return;
    }
#if 0
    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
    ALOGV("setDataSourceFD: fd %d", fd);
    mp->setDataSource(fd, offset, length);
#endif
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

static void
jni_dttv_setLooping(JNIEnv *env, jobject thiz, jboolean looping) {
    return;
}

static jboolean
jni_dttv_isLooping(JNIEnv *env, jobject thiz) {
    return false;
}

static void
jni_dttv_setVolume(JNIEnv *env, jobject thiz, float leftVolume, float rightVolume) {
    return;
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

static jboolean
jni_dttv_getTrackInfo(JNIEnv *env, jobject thiz, jobject sparse) {
    DTPlayer *media_player = getMediaPlayer(env, thiz);
    if (media_player == NULL) {
        return false;
    }

    dtp_media_info_t *info = media_player->getMediaInfo();
    int MEDIA_TRACK_TYPE_UNKNOWN = 0;
    int MEDIA_TRACK_TYPE_VIDEO = 1;
    int MEDIA_TRACK_TYPE_AUDIO = 2;
    int MEDIA_TRACK_TYPE_TIMEDTEXT = 3;
    int MEDIA_TRACK_TYPE_SUBTITLE = 4;

    char buf[1024];

    jclass sparse_class = env->GetObjectClass(sparse);
    //jmethodID sparse_init = env->GetMethodID(sparse_class, "<init>", "()V");
    //jobject sparse = env->NewObject(sparse_class, sparse_init);

    jmethodID put = env->GetMethodID(
            sparse_class,
            "put",
            "(ILjava/lang/Object;)V"
    );

    jclass string_class = env->FindClass("java/lang/String");
    jstring string_encode = env->NewStringUTF("utf-8");
    jmethodID string_getBytes = env->GetMethodID(string_class, "getBytes",
                                                 "(Ljava/lang/String;)[B");

    char *lang = "und";
    int i = 0;
    char index[10];
    int count = 0;


    if (info->has_video) {
        buf[0] = '\0';
        for (i = 0; i < info->tracks.vst_num; i++) {
            if (i > 0)
                strcat(buf, "!#!");
            sprintf(index, "%d", count++);
            strcat(buf, index);
            strcat(buf, ".");
            strcat(buf, info->tracks.vstreams[i]->language);
        }

        jbyteArray value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                              string_getBytes, string_encode);
        env->CallVoidMethod(sparse, put, MEDIA_TRACK_TYPE_VIDEO, value);
        LOGV("video trackinfo: %s\n", buf);
    }

    if (info->has_audio) {
        buf[0] = '\0';
        for (i = 0; i < info->tracks.ast_num; i++) {
            if (i > 0)
                strcat(buf, "!#!");
            sprintf(index, "%d", count++);
            strcat(buf, index);
            strcat(buf, ".");
            strcat(buf, info->tracks.astreams[i]->language);
        }

        jbyteArray value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                              string_getBytes, string_encode);
        env->CallVoidMethod(sparse, put, MEDIA_TRACK_TYPE_AUDIO, value);
        LOGV("audio trackinfo: %s\n", buf);
    }

    if (info->has_sub) {
        buf[0] = '\0';
        for (i = 0; i < info->tracks.sst_num; i++) {
            if (i > 0)
                strcat(buf, "!#!");
            sprintf(index, "%d", count++);
            strcat(buf, index);
            strcat(buf, ".");
            strcat(buf, info->tracks.sstreams[i]->language);
        }

        jbyteArray value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                              string_getBytes, string_encode);
        env->CallVoidMethod(sparse, put, MEDIA_TRACK_TYPE_SUBTITLE, value);
        LOGV("sub trackinfo: %s\n", buf);
    }

    env->DeleteLocalRef(sparse_class);
    env->DeleteLocalRef(string_class);

    return true;
}

jstring jni_dttv_getTimedTextPath() {
    return NULL;
}

static jboolean
jni_dttv_getMetadata(JNIEnv *env, jobject thiz, jobject meta) {
    DTPlayer *media_player = getMediaPlayer(env, thiz);
    if (media_player == NULL) {
        return false;
    }

    dtp_media_info_t *info = media_player->getMediaInfo();

    jobject obj = env->NewLocalRef(meta);
    //jclass clazz = env->FindClass("java/util/HashMap");
    jclass clazz = env->GetObjectClass(obj);
    jmethodID init = env->GetMethodID(clazz, "<init>", "()V");
    //jobject hashmap = env->NewObject(clazz, init);

    jmethodID put = env->GetMethodID(
            clazz,
            "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"
    );

    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("utf-8");
    jmethodID getBytes = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");

    //env->CallObjectMethod(hashmap, put, env->NewStringUTF("title"), env->NewStringUTF("video"));
    //env->CallObjectMethod(hashmap, put, env->NewStringUTF("duration"), env->NewStringUTF("1000"));

    //env->CallObjectMethod(meta, put, env->NewStringUTF("title"), env->NewStringUTF("video"));
    //env->CallObjectMethod(meta, put, env->NewStringUTF("duration"), env->NewStringUTF("1000"));

    char buf[1024];
    //-------------------------------------------Media----------------------------------------------
    // title
    sprintf(buf, "%s", info->file);
    jbyteArray title_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("title"), getBytes,
                                                              strencode);
    jbyteArray title_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                getBytes, strencode);
    env->CallObjectMethod(meta, put, title_key, title_value);
    // fomat
    sprintf(buf, "%d", info->format);
    jbyteArray fmt_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("format"), getBytes,
                                                            strencode);
    jbyteArray fmt_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                              getBytes, strencode);
    env->CallObjectMethod(meta, put, fmt_key, fmt_value);
    // bitrate
    sprintf(buf, "%d", info->bit_rate);
    jbyteArray br_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("bitrate"), getBytes,
                                                           strencode);
    jbyteArray br_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                             getBytes, strencode);
    env->CallObjectMethod(meta, put, br_key, br_value);
    // filesize
    sprintf(buf, "%d", info->file_size);
    jbyteArray fs_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("length"), getBytes,
                                                           strencode);
    jbyteArray fs_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                             getBytes, strencode);
    env->CallObjectMethod(meta, put, fs_key, fs_value);
    // filesize
    sprintf(buf, "%d", info->tracks.ast_num + info->tracks.vst_num + info->tracks.sst_num);
    jbyteArray nt_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("num_tracks"),
                                                           getBytes,
                                                           strencode);
    jbyteArray nt_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                             getBytes, strencode);
    env->CallObjectMethod(meta, put, nt_key, nt_value);
    // duration
    sprintf(buf, "%d", media_player->getDuration());
    jbyteArray dur_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("duration"), getBytes,
                                                            strencode);
    jbyteArray dur_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf), getBytes,
                                                              strencode);
    env->CallObjectMethod(meta, put, dur_key, dur_value);

    if (media_player->getDuration() > 0) {
        sprintf(buf, "%s", "true");
        jbyteArray pause_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("cap_pause"),
                                                                  getBytes,
                                                                  strencode);
        jbyteArray pause_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                    getBytes,
                                                                    strencode);
        env->CallObjectMethod(meta, put, pause_key, pause_value);

        sprintf(buf, "%s", "true");
        jbyteArray seek_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("cap_seek"),
                                                                 getBytes,
                                                                 strencode);
        jbyteArray seek_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf), getBytes,
                                                                   strencode);
        env->CallObjectMethod(meta, put, seek_key, seek_value);
    }

    //-------------------------------------------Audio----------------------------------------------
    if (info->has_audio) {
        int cur = info->cur_ast_index;
        sprintf(buf, "%d", info->tracks.astreams[cur]->format);
        jbyteArray ac_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("audio_codec"),
                                                               getBytes,
                                                               strencode);
        jbyteArray ac_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                 getBytes, strencode);
        env->CallObjectMethod(meta, put, ac_key, ac_value);

        sprintf(buf, "%d", info->tracks.astreams[cur]->sample_rate);
        jbyteArray sr_key = (jbyteArray) env->CallObjectMethod(
                env->NewStringUTF("audio_sample_rate"),
                getBytes,
                strencode);
        jbyteArray sr_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                 getBytes, strencode);
        env->CallObjectMethod(meta, put, sr_key, sr_value);


    }
    //-------------------------------------------Video----------------------------------------------
    if (info->has_video) {
        int cur = info->cur_vst_index;
        sprintf(buf, "%d", info->tracks.vstreams[cur]->format);
        jbyteArray vc_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("video_codec"),
                                                               getBytes,
                                                               strencode);
        jbyteArray vc_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                 getBytes, strencode);
        env->CallObjectMethod(meta, put, vc_key, vc_value);

        sprintf(buf, "%d", info->tracks.vstreams[cur]->width);
        jbyteArray vw_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("video_width"),
                                                               getBytes,
                                                               strencode);
        jbyteArray vw_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                 getBytes, strencode);
        env->CallObjectMethod(meta, put, vw_key, vw_value);

        sprintf(buf, "%d", info->tracks.vstreams[cur]->height);
        jbyteArray vh_key = (jbyteArray) env->CallObjectMethod(env->NewStringUTF("video_height"),
                                                               getBytes,
                                                               strencode);
        jbyteArray vh_value = (jbyteArray) env->CallObjectMethod(env->NewStringUTF(buf),
                                                                 getBytes, strencode);
        env->CallObjectMethod(meta, put, vh_key, vh_value);
    }
    //--------------------------------------------Sub-----------------------------------------------

    env->DeleteLocalRef(clsstring);
    env->DeleteLocalRef(clazz);

    return true;
}

void
jni_dttv_addTimedTextSource(JNIEnv *env, jobject thiz, jstring path) {
    return;
}

void
jni_dttv_selectOrDeselectTrack(JNIEnv *env, jobject thiz, jint index, jboolean select) {
    return;
}

int jni_dttv_get_parameter(JNIEnv *env, jobject thiz, int cmd, jlong arg1, jlong arg2) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return -1;
    }
    return 0;
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
        case KEY_PARAMETER_SET_GLFILTER:
            gl_set_parameter(KEY_PARAMETER_SET_GLFILTER, arg1, arg2);
            break;
        case KEY_PARAMETER_GLRENDER_SET_FILTER_PARAMETER:
            gl_set_parameter(KEY_PARAMETER_GLRENDER_SET_FILTER_PARAMETER, arg1, arg2);
            break;
        default:
            break;
    }
    return 0;
}

int jni_dttv_set_gl_parameter(JNIEnv *env, jobject thiz, int cmd, jintArray j_array) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return -1;
    }

    jint i, sum = 0;
    jint *c_array;
    jint arr_len;
    //1. 获取数组长度
    arr_len = env->GetArrayLength(j_array);
    //2. 根据数组长度和数组元素的数据类型申请存放java数组元素的缓冲区
    c_array = (jint *) malloc(sizeof(jint) * arr_len);
    //3. 初始化缓冲区
    memset(c_array, 0, sizeof(jint) * arr_len);
    printf("arr_len = %d ", arr_len);
    //4. 拷贝Java数组中的所有元素到缓冲区中
    env->GetIntArrayRegion(j_array, 0, arr_len, c_array);
    //5. Handle parameter
    gl_set_parameter(KEY_PARAMETER_GLRENDER_SET_FILTER_PARAMETER, (unsigned long) c_array, 0);
    free(c_array);  //6. 释放存储数组元素的缓冲区
    return 0;
}

jstring jni_dttv_getMetaEncoding(JNIEnv *env, jobject thiz) {
    char msg[60] = "UTF-8";
    return env->NewStringUTF(msg);
}

void jni_dttv_setMetaEncoding(JNIEnv *env, jobject thiz, jstring encoding) {
    return;
}

static void jni_dttv_set_video_surface(JNIEnv *env, jobject thiz, jobject jsurface) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return;
    }
    //ANativeWindow *window = ANativeWindow_fromSurface(env, jsurface);
    //LOGV("NativeWindow Address:%p \n", window);
    //mp->setNativeWindow(window);
    jobject ref = env->NewGlobalRef(jsurface);
    mp->setSurface(ref);
}

int jni_gl_surface_create(JNIEnv *env, jobject thiz) {

    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return 0;
    }
    gl_create((void *) getMediaPlayer(env, thiz));
    mp->setGLSurfaceView();
    return 0;
}

int jni_gl_surface_change(JNIEnv *env, jobject thiz, int w, int h) {
    DTPlayer *mp = getMediaPlayer(env, thiz);
    if (mp == NULL) {
        LOGV("set parameter failed.mp == null.");
        return 0;
    }
    gl_setup(w, h);
    LOGV("on surface changed, w:%d h:%d \n", w, h);
    return 0;
}


int jni_gl_draw_frame(JNIEnv *env, jobject thiz) {
    gl_render();
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
        {"native_init",                 "()V",                                                         (void *) jni_dttv_init},
        {"native_setup",                "(Ljava/lang/Object;)I",                                       (void *) jni_dttv_setup},
        {"native_release",              "()I",                                                         (void *) jni_dttv_release},
        {"native_finalize",             "()V",                                                         (void *) jni_dttv_native_finalize},
        {"native_set_datasource",       "(Ljava/lang/String;[Ljava/lang/String;[Ljava/lang/String;)V", (void *) jni_dttv_setDataSourceAndHeaders},
        {"native_set_datasource",       "(Ljava/io/FileDescriptor;JJ)V",                               (void *) jni_dttv_setDataSourceFD},
        {"native_prepare",              "()I",                                                         (void *) jni_dttv_prepare},
        {"native_prepare_async",        "()I",                                                         (void *) jni_dttv_prepareasync},
        {"native_start",                "()I",                                                         (void *) jni_dttv_start},
        {"native_pause",                "()I",                                                         (void *) jni_dttv_pause},
        {"native_seekTo",               "(I)I",                                                        (void *) jni_dttv_seekTo},
        {"native_stop",                 "()I",                                                         (void *) jni_dttv_stop},
        {"native_reset",                "()I",                                                         (void *) jni_dttv_reset},
        {"setLooping",                  "(Z)V",                                                        (void *) jni_dttv_setLooping},
        {"isLooping",                   "()Z",                                                         (void *) jni_dttv_isLooping},
        {"setVolume",                   "(FF)V",                                                       (void *) jni_dttv_setVolume},
        {"native_get_video_width",      "()I",                                                         (void *) jni_dttv_get_video_width},
        {"native_get_video_height",     "()I",                                                         (void *) jni_dttv_get_video_height},
        {"native_is_playing",           "()I",                                                         (void *) jni_dttv_is_playing},
        {"native_get_current_position", "()I",                                                         (void *) jni_dttv_get_current_position},
        {"native_get_duration",         "()I",                                                         (void *) jni_dttv_get_duration},
        {"native_getTrackInfo",         "(Landroid/util/SparseArray;)Z",                               (void *) jni_dttv_getTrackInfo},
        {"getTimedTextPath",            "()Ljava/lang/String;",                                        (void *) jni_dttv_getTimedTextPath},

        {"native_getMetadata",          "(Ljava/util/Map;)Z",                                          (void *) jni_dttv_getMetadata},
        {"addTimedTextSource",          "(Ljava/lang/String;)V",                                       (void *) jni_dttv_addTimedTextSource},
        {"selectOrDeselectTrack",       "(IZ)V",                                                       (void *) jni_dttv_selectOrDeselectTrack},
        {"native_get_parameter",        "(IJJ)I",                                                      (void *) jni_dttv_get_parameter},
        {"native_set_parameter",        "(IJJ)I",                                                      (void *) jni_dttv_set_parameter},
        {"native_set_gl_parameter",     "(I[I)I",                                                      (void *) jni_dttv_set_gl_parameter},
        {"getMetaEncoding",             "()Ljava/lang/String;",                                        (void *) jni_dttv_getMetaEncoding},
        {"setMetaEncoding",             "(Ljava/lang/String;)V",                                       (void *) jni_dttv_setMetaEncoding},
        {"native_set_video_surface",    "(Landroid/view/Surface;)V",                                   (void *) jni_dttv_set_video_surface},
        {"native_surface_create",       "()I",                                                         (void *) jni_gl_surface_create},
        {"native_surface_change",       "(II)I",                                                       (void *) jni_gl_surface_change},
        {"native_draw_frame",           "()I",                                                         (void *) jni_gl_draw_frame},

        {"native_set_audio_effect",     "(I)I",                                                        (void *) jni_dttv_set_audio_effect},
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
    java_vm = vm;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        LOGV("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_natives(env) < 0) {
        LOGV("ERROR: MediaPlayer native registration failed\n");
        goto bail;
    }

    result = JNI_VERSION_1_6;
    lock_init(&mutex, NULL);
    av_jni_set_java_vm(vm, reserved);
    dttv_setup_gvm(java_vm);
    bail:
    return result;
}
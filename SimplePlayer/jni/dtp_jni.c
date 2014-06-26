#include <jni.h>
#include <string.h>
#include <android/log.h>

#include "dtplayer_api.h"
#include "render_android.h"

#define DEBUG_TAG "dttvJni"

static void *handle;
extern int android_audio_init(int channels, int samplerate);

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
    char * file_name = (*env)->GetStringUTFChars(env, url, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Receive start cmd, file [%s] size:%d ",file_name,strlen(file_name));


    render_init();

    dtplayer_para_t para;

    para.no_audio = para.no_video = para.no_sub = -1;
	para.height = para.width = -1;
	para.loop_mode = 0;
	para.audio_index = para.video_index = para.sub_index = -1;
	para.update_cb = NULL;
	para.sync_enable = -1;

	para.file_name = file_name;
	//para.update_cb = (void *) update_cb;
	//para.no_audio=1;
	para.no_video=1;
	para.width = 720;
	para.height = 480;

    handle = dtplayer_init(&para);
    if (!handle)
        return -1;

    //get media info
	dt_media_info_t info;
	int ret = dtplayer_get_mediainfo(handle, &info);
	if(ret < 0)
	{
	    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Get mediainfo failed, quit \n");
		return -1;
	}
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Get Media Info Ok,filesize:%lld fulltime:%lld S \n",info.file_size,info.duration);

	//dtplayer_start (handle);
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
    if(handle)
    	dtplayer_stop (handle);
    handle = NULL;
    return 0;
}

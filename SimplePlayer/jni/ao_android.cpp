#include <jni.h>
#include <string.h>
#include <android/log.h>

#include <media/AudioTrack.h>

extern "C"{

#include "dtaudio_android.h"

}

namespace android {

extern "C" {

#define TAG "AO-ANDROID"
ao_wrapper_t ao_android_ops;
static char *ao_android_name = "android ao";


static int android_audio_init(int channels, int samplerate)
{
   AudioTrack* atrack = new AudioTrack();
   __android_log_print(ANDROID_LOG_INFO,TAG,"AudioTrack created at %p. Now trying to setup", atrack);
   __android_log_print(ANDROID_LOG_INFO,TAG,"channel:%d samplerate:%d", channels,samplerate);
   return 0;
}

static int android_audio_pause()
{
   return 0;
}

static int android_audio_resume()
{
   return 0;
}

static int android_audio_play()
{
   return 0;
}

static int android_audio_level()
{
   return 0;
}

static int android_audio_latency()
{
   return 0;
}

static int android_audio_stop()
{
   return 0;
}

void android_ops_init()
{
	ao_android_ops.id = 0x100;
    ao_android_ops.name = ao_android_name;
#if 0
    ao_android_ops.ao_init = android_audio_init;
    ao_android_ops.ao_pause = android_audio_pause;
    ao_android_ops.ao_resume = android_audio_resume;
    ao_android_ops.ao_stop = android_audio_stop;
    ao_android_ops.ao_write = android_audio_play;
    ao_android_ops.ao_level = android_audio_level;
    ao_android_ops.ao_latency = android_audio_latency;
#endif
}
}

}

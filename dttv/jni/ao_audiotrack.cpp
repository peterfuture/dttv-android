#include <jni.h>
#include <string.h>
#include <android/log.h>

#include <media/AudioTrack.h>
//#include "/home/shihx1/androidSystem/frameworks/av/include/media/AudioTrack.h"

extern "C"{

#include "dtaudio_android.h"
#include "dt_buffer.h"

}

namespace android {

extern "C" {

#define TAG "AO-ANDROID"

typedef struct{
	sp<AudioTrack> sptrack;
    int audio_started;
    dt_buffer_t dbt;
}android_ao_ctx_t;

ao_wrapper_t ao_android_ops;
static char *ao_android_name = "ao android";
static ao_wrapper_t *wrapper = &ao_android_ops;

static void audioCallback(int event, void* user, void *info)
{
    ao_wrapper_t *wrapper = (ao_wrapper_t *)user;
    android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;

    AudioTrack::Buffer *buffer = static_cast<AudioTrack::Buffer *>(info);
    if (event != AudioTrack::EVENT_MORE_DATA) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"audioCallback: event = %d \n", event);
        return;
    }

    if (buffer == NULL || buffer->size == 0) {
    	__android_log_print(ANDROID_LOG_DEBUG,TAG,"audioCallback: Wrong buffer\n");
        return;
    }

    if(buf_level(&ctx->dbt) < buffer->size)
    {
    	buffer->size = 0;
    	return;
    }
    buf_get(&ctx->dbt,(uint8_t *)buffer->i16,buffer->size);
    ctx->audio_started = 1;
    return;
}

static int android_audio_init(dtaudio_output_t *aout, dtaudio_para_t *para)
{
	int ret = 0;
	status_t status;
    sp<AudioTrack> track;
    int session_id;
    
    uint32_t afSampleRate;
    size_t afFrameCount;
    uint32_t frameCount;
    int bufferCount = 4;

	memcpy(&wrapper->para,para,sizeof(dtaudio_para_t));
	android_ao_ctx_t *ctx = (android_ao_ctx_t *)malloc(sizeof(*ctx));
	if(!ctx)
	{
		__android_log_print(ANDROID_LOG_DEBUG,TAG,"SDL CTX MALLOC FAILED \n");
		return -1;
	}
	memset(ctx,0,sizeof(*ctx));
	if(buf_init(&ctx->dbt,para->dst_samplerate * 4 / 10) < 0) // 100ms
	{
		ret = -1;
		goto FAIL;
	}
	wrapper->ao_priv = ctx;
    
    if (AudioSystem::getOutputFrameCount(&afFrameCount, AUDIO_STREAM_MUSIC) != NO_ERROR) {
        return NO_INIT;
    }
    if (AudioSystem::getOutputSamplingRate(&afSampleRate, AUDIO_STREAM_MUSIC) != NO_ERROR) {
        return NO_INIT;
    }
    frameCount = (para->dst_samplerate*afFrameCount*bufferCount)/afSampleRate;
    
    session_id = AudioSystem::newAudioSessionId();
    track = new AudioTrack(
                AUDIO_STREAM_MUSIC,
                para->dst_samplerate,
                AUDIO_FORMAT_PCM_16_BIT,
                (para->dst_channels == 1) ? AUDIO_CHANNEL_OUT_MONO : AUDIO_CHANNEL_OUT_STEREO,
                frameCount, // frameCount
                AUDIO_OUTPUT_FLAG_NONE,
                audioCallback,
                wrapper,
                0, // notification frames
                session_id, // mSessionID
                AudioTrack::TRANSFER_CALLBACK,
                NULL, // offload info
                0  // mUID 
            );
    
    if(track == 0 || track->initCheck() != NO_ERROR)
    {
        __android_log_print(ANDROID_LOG_DEBUG,TAG,"Android Audiotrack new failed \n");
        goto FAIL;
    }
	track->start();
    ctx->sptrack = track;
	__android_log_print(ANDROID_LOG_DEBUG,TAG,"Android AO Init OK\n");
	return 0;
FAIL:
	buf_release(&ctx->dbt);
	free(ctx);
	wrapper->ao_priv = NULL;
	return ret;
}

static int android_audio_pause(dtaudio_output_t *aout)
{
	__android_log_print(ANDROID_LOG_DEBUG,TAG,"android out pause");

	android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;
	if(ctx->sptrack != 0)
        ctx->sptrack->pause();
   	return 0;
}

static int android_audio_resume(dtaudio_output_t *aout)
{
	__android_log_print(ANDROID_LOG_DEBUG,TAG,"android out pause");

	android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;
	if(ctx->sptrack != 0)
        ctx->sptrack->start();
	return 0;
}

static int android_audio_play(dtaudio_output_t *aout, uint8_t * buf, int size)
{
	android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;
	return buf_put(&ctx->dbt,buf,size);
}

static int android_audio_level(dtaudio_output_t *aout)
{
	android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;
    return ctx->dbt.level;
}

static int64_t android_audio_latency(dtaudio_output_t *aout)
{
	android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;
    if(ctx->sptrack == 0)
        return 0;

    if(ctx->audio_started == 0)
        return 0;
	int level = buf_level(&ctx->dbt);
	unsigned int sample_num;
	uint64_t latency;
	float pts_ratio = 0.0;
	pts_ratio = (double) 90000 / wrapper->para.dst_samplerate;
	sample_num = level / (wrapper->para.dst_channels * wrapper->para.bps / 8);
	latency = (sample_num * pts_ratio) + (int64_t)ctx->sptrack->latency();
	return latency;
}

static int android_audio_stop(dtaudio_output_t *aout)
{
	if(wrapper->ao_priv)
	{
		android_ao_ctx_t *ctx = (android_ao_ctx_t *)wrapper->ao_priv;
		if(ctx->sptrack != 0)
        {
            ctx->sptrack->stop();
            ctx->sptrack.clear();
        }
		buf_release(&ctx->dbt);
		free(ctx);
		wrapper->ao_priv = NULL;
	}
	return 0;
}

void ao_audiotrack_init()
{
	ao_android_ops.id = 0x100;//AO_ID_ANDROID
    ao_android_ops.name = ao_android_name;
    ao_android_ops.ao_init = android_audio_init;
    ao_android_ops.ao_pause = android_audio_pause;
    ao_android_ops.ao_resume = android_audio_resume;
    ao_android_ops.ao_stop = android_audio_stop;
    ao_android_ops.ao_write = android_audio_play;
    ao_android_ops.ao_level = android_audio_level;
    ao_android_ops.ao_latency = android_audio_latency;
}
}

}

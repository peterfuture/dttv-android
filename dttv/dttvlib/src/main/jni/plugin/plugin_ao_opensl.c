#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>
#include <android/log.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <dtp_audio_plugin.h>
#include <dtp_av.h>
#include "../dttv_jni_utils.h"
#include "../dttv_jni_log.h"

#define OPENSLES_BUFFERS 255 /* maximum number of buffers */
#define OPENSLES_BUFLEN  10   /* ms */

#define TAG "AO-OPENSL"

/*
 * 10ms of precision when mesasuring latency should be enough,
 * with 255 buffers we can buffer 2.55s of audio.
 */

#define CHECK_OPENSL_ERROR(msg)                \
    if ((result != SL_RESULT_SUCCESS))         \
    {                                          \
        LOGI("%s", msg);                  \
        goto error;                            \
    }

typedef SLresult (*slCreateEngine_t)(
        SLObjectItf *, SLuint32, const SLEngineOption *, SLuint32,
        const SLInterfaceID *, const SLboolean *);

#define Destroy(a) (*a)->Destroy(a);
#define SetPlayState(a, b) (*a)->SetPlayState(a, b)
#define RegisterCallback(a, b, c) (*a)->RegisterCallback(a, b, c)
#define GetInterface(a, b, c) (*a)->GetInterface(a, b, c)
#define Realize(a, b) (*a)->Realize(a, b)
#define CreateOutputMix(a, b, c, d, e) (*a)->CreateOutputMix(a, b, c, d, e)
#define CreateAudioPlayer(a, b, c, d, e, f, g) \
    (*a)->CreateAudioPlayer(a, b, c, d, e, f, g)
#define Enqueue(a, b, c) (*a)->Enqueue(a, b, c)
#define Clear(a) (*a)->Clear(a)
#define GetState(a, b) (*a)->GetState(a, b)
#define SetPositionUpdatePeriod(a, b) (*a)->SetPositionUpdatePeriod(a, b)
#define SetVolumeLevel(a, b) (*a)->SetVolumeLevel(a, b)
#define SetMute(a, b) (*a)->SetMute(a, b)


//From vlc
#define CLOCK_FREQ INT64_C(1000000)
#define AOUT_MAX_ADVANCE_TIME           (AOUT_MAX_PREPARE_TIME + CLOCK_FREQ)
#define AOUT_MAX_PREPARE_TIME           (2 * CLOCK_FREQ)
#define AOUT_MIN_PREPARE_TIME           AOUT_MAX_PTS_ADVANCE
#define AOUT_MAX_PTS_ADVANCE            (CLOCK_FREQ / 25)
#define AOUT_MAX_PTS_DELAY              (3 * CLOCK_FREQ / 50)
#define AOUT_MAX_RESAMPLING             10


/*****************************************************************************
 *
 *****************************************************************************/

typedef struct aout_sys_t {
    /* OpenSL objects */
    SLObjectItf engineObject;
    SLObjectItf outputMixObject;
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    SLObjectItf playerObject;
    SLVolumeItf volumeItf;
    SLEngineItf engineEngine;
    SLPlayItf playerPlay;

    /* OpenSL symbols */
    void *p_so_handle;

    slCreateEngine_t slCreateEnginePtr;
    SLInterfaceID SL_IID_ENGINE;
    SLInterfaceID SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
    SLInterfaceID SL_IID_VOLUME;
    SLInterfaceID SL_IID_PLAY;

    /* audio buffered through opensles */
    uint8_t *buf;
    int samples_per_buf;
    int next_buf;

    int rate;

    /* if we can measure latency already */
    int started;
    int samples;
    dt_buffer_t dbt;
    lock_t lock;
} aout_sys_t;


//====================================
// dtap
//====================================

#ifdef ENABLE_DTAP

#include "dtap_api.h"
typedef struct{
    dtap_context_t ap;
    lock_t lock;
}audio_effect_t;

#endif



/*****************************************************************************
 *
 *****************************************************************************/

static inline int bytesPerSample(ao_context_t *aoc) {
    dtaudio_para_t *para = &aoc->para;
    return para->dst_channels * para->data_width / 8;
    //return 2 /* S16 */ * 2 /* stereo */;
}

// get us delay
//
static int TimeGet(ao_context_t *aoc, int64_t *drift) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;

    SLAndroidSimpleBufferQueueState st;
    SLresult res = GetState(sys->playerBufferQueue, &st);
    if ((res != SL_RESULT_SUCCESS)) {
        LOGV("Could not query buffer queue state in TimeGet (%lu)", (unsigned long) res);
        return -1;
    }

    lock(&sys->lock);
    bool started = sys->started;
    unlock(&sys->lock);

    if (!started)
        return -1;
    *drift = (CLOCK_FREQ * OPENSLES_BUFLEN * st.count / 1000)
             + sys->samples * CLOCK_FREQ / sys->rate;

    //__android_log_print(ANDROID_LOG_DEBUG, TAG, "latency %lld ms, %d/%d buffers, samples:%d", *drift / 1000,
    //        (int)st.count, OPENSLES_BUFFERS, sys->samples);

    return 0;
}

static void Flush(ao_context_t *aoc, bool drain) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;

    if (drain) {
        int64_t delay;
        if (!TimeGet(aoc, &delay))
            usleep(delay * 1000);
    } else {
        SetPlayState(sys->playerPlay, SL_PLAYSTATE_STOPPED);
        Clear(sys->playerBufferQueue);
        SetPlayState(sys->playerPlay, SL_PLAYSTATE_PLAYING);

        sys->samples = 0;
        sys->started = 0;
    }
}

static void Pause(ao_context_t *aoc, bool pause) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    SetPlayState(sys->playerPlay,
                 pause ? SL_PLAYSTATE_PAUSED : SL_PLAYSTATE_PLAYING);
}

static int WriteBuffer(ao_context_t *aoc) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    const int unit_size = sys->samples_per_buf * bytesPerSample(aoc);

    /* Check if we can fill at least one buffer unit by chaining blocks */
    if (sys->dbt.level < unit_size) {
        return false;
    }

    SLAndroidSimpleBufferQueueState st;
    SLresult res = GetState(sys->playerBufferQueue, &st);
    if ((res != SL_RESULT_SUCCESS)) {
        return false;
    }

    if (st.count == OPENSLES_BUFFERS)
        return false;

    int done = 0;
    while (done < unit_size) {
        int cur = buf_level(&sys->dbt);
        if (cur > unit_size - done)
            cur = unit_size - done;

        //memcpy(&sys->buf[unit_size * sys->next_buf + done], b->p_buffer, cur);
        buf_get(&sys->dbt, &sys->buf[unit_size * sys->next_buf + done], cur);
        done += cur;

        if (done == unit_size)
            break;
    }

    SLresult r = Enqueue(sys->playerBufferQueue,
                         &sys->buf[unit_size * sys->next_buf], unit_size);

    sys->samples -= sys->samples_per_buf;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "minus sampels, %d minus %d \n",sys->samples, sys->samples_per_buf);

    if (r == SL_RESULT_SUCCESS) {
        if (++sys->next_buf == OPENSLES_BUFFERS)
            sys->next_buf = 0;
        return true;
    } else {
        /* XXX : if writing fails, we don't retry */
        return false;
    }
}

/*****************************************************************************
 * Play: play a sound
 *****************************************************************************/
static int Play(ao_context_t *aoc, uint8_t *buf, int size) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    int ret = 0;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "space:%d level:%d  size:%d  \n",buf_space(&sys->dbt), buf_level(&sys->dbt), size);
    if (buf_space(&sys->dbt) > size) {
        ret = buf_put(&sys->dbt, buf, size);
    }
    sys->samples += ret / bytesPerSample(aoc);
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "add sampels, %d add %d \n",sys->samples, ret / bytesPerSample(aout));

    /* Fill OpenSL buffer */
    WriteBuffer(aoc); // will read data in callback
    return ret;
}

static void PlayedCallback(SLAndroidSimpleBufferQueueItf caller, void *pContext) {
    (void) caller;
    ao_context_t *aoc = (ao_context_t *)pContext;
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;

    assert (caller == sys->playerBufferQueue);
    sys->started = 1;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "opensl callback called \n");

}

/*****************************************************************************
 *
 *****************************************************************************/

static SLuint32 convertSampleRate(SLuint32 sr) {
    switch (sr) {
        case 8000:
            return SL_SAMPLINGRATE_8;
        case 11025:
            return SL_SAMPLINGRATE_11_025;
        case 12000:
            return SL_SAMPLINGRATE_12;
        case 16000:
            return SL_SAMPLINGRATE_16;
        case 22050:
            return SL_SAMPLINGRATE_22_05;
        case 24000:
            return SL_SAMPLINGRATE_24;
        case 32000:
            return SL_SAMPLINGRATE_32;
        case 44100:
            return SL_SAMPLINGRATE_44_1;
        case 48000:
            return SL_SAMPLINGRATE_48;
        case 64000:
            return SL_SAMPLINGRATE_64;
        case 88200:
            return SL_SAMPLINGRATE_88_2;
        case 96000:
            return SL_SAMPLINGRATE_96;
        case 192000:
            return SL_SAMPLINGRATE_192;
    }
    return -1;
}

static int Start(ao_context_t *aoc) {
    SLresult result;

    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    dtaudio_para_t *para = &aoc->para;

    // configure audio source - this defines the number of samples you can enqueue.
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            OPENSLES_BUFFERS
    };

    int mask;

    if (para->dst_channels > 1)
        mask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    else
        mask = SL_SPEAKER_FRONT_CENTER;


    SLDataFormat_PCM format_pcm;
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = para->dst_channels;
    //format_pcm.samplesPerSec    = ((SLuint32) para->dst_samplerate * 1000) ;
    format_pcm.samplesPerSec = ((SLuint32) convertSampleRate(para->dst_samplerate));
    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.channelMask = mask;
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {
            SL_DATALOCATOR_OUTPUTMIX,
            sys->outputMixObject
    };
    SLDataSink audioSnk = {&loc_outmix, NULL};

    //create audio player
    const SLInterfaceID ids2[] = {sys->SL_IID_ANDROIDSIMPLEBUFFERQUEUE, sys->SL_IID_VOLUME};
    static const SLboolean req2[] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = CreateAudioPlayer(sys->engineEngine, &sys->playerObject, &audioSrc,
                               &audioSnk, sizeof(ids2) / sizeof(*ids2),
                               ids2, req2);
    if ((result != SL_RESULT_SUCCESS)) { // error
        return -1;
        /* Try again with a more sensible samplerate */
#if 0
        fmt->i_rate = 44100;
        format_pcm.samplesPerSec = ((SLuint32) 44100 * 1000) ;
        result = CreateAudioPlayer(sys->engineEngine, &sys->playerObject, &audioSrc,
                &audioSnk, sizeof(ids2) / sizeof(*ids2),
                ids2, req2);
#endif
    }
    CHECK_OPENSL_ERROR("Failed to create audio player");

    result = Realize(sys->playerObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR("Failed to realize player object.");

    result = GetInterface(sys->playerObject, sys->SL_IID_PLAY, &sys->playerPlay);
    CHECK_OPENSL_ERROR("Failed to get player interface.");

    result = GetInterface(sys->playerObject, sys->SL_IID_VOLUME, &sys->volumeItf);
    CHECK_OPENSL_ERROR("failed to get volume interface.");

    result = GetInterface(sys->playerObject, sys->SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                          &sys->playerBufferQueue);
    CHECK_OPENSL_ERROR("Failed to get buff queue interface");

    result = RegisterCallback(sys->playerBufferQueue, PlayedCallback,
                              (void *) aoc);
    CHECK_OPENSL_ERROR("Failed to register buff queue callback.");

    // set the player's state to playing
    result = SetPlayState(sys->playerPlay, SL_PLAYSTATE_PLAYING);
    CHECK_OPENSL_ERROR("Failed to switch to playing state");

    /* XXX: rounding shouldn't affect us at normal sampling rate */
    sys->rate = para->dst_samplerate;
    sys->samples_per_buf = OPENSLES_BUFLEN * para->dst_samplerate / 1000;
    sys->buf = (uint8_t *)malloc(OPENSLES_BUFFERS * sys->samples_per_buf * bytesPerSample(aoc));
    if (!sys->buf)
        goto error;

    sys->started = 0;
    sys->next_buf = 0;

    sys->samples = 0;
    SetPositionUpdatePeriod(sys->playerPlay, AOUT_MIN_PREPARE_TIME * 1000 / CLOCK_FREQ);
    return 0;

    error:
    if (sys->playerObject) {
        Destroy(sys->playerObject);
        sys->playerObject = NULL;
    }

    return -1;
}

static void Stop(ao_context_t *aoc) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;

    SetPlayState(sys->playerPlay, SL_PLAYSTATE_STOPPED);
    //Flush remaining buffers if any.
    Clear(sys->playerBufferQueue);
    Destroy(sys->playerObject);
    Destroy(sys->outputMixObject);
    Destroy(sys->engineObject);
    dlclose(sys->p_so_handle);

    free(sys->buf);
    free(sys);
    sys = NULL;
}

/*****************************************************************************
 *
 *****************************************************************************/

static int Open(ao_context_t *aoc) {

    dtaudio_para_t *para = &aoc->para;
    SLresult result;

    struct aout_sys_t *sys = (struct aout_sys_t *) malloc(sizeof(*sys));
    if (sys == NULL)
        return -1;
    memset(sys, 0, sizeof(struct aout_sys_t));
    sys->p_so_handle = dlopen("libOpenSLES.so", RTLD_NOW);
    if (sys->p_so_handle == NULL) {
        goto error;
    }

    sys->slCreateEnginePtr = (slCreateEngine_t)dlsym(sys->p_so_handle, "slCreateEngine");
    if (sys->slCreateEnginePtr == NULL) {
        goto error;
    }

#define OPENSL_DLSYM(dest, name)                       \
    do {                                                       \
        const SLInterfaceID *sym = (SLInterfaceID *)dlsym(sys->p_so_handle, "SL_IID_" name);        \
        if ((sym == NULL))                             \
        {                                                      \
            goto error;                                        \
        }                                                      \
        sys->dest = *sym;                                           \
    } while(0)

    OPENSL_DLSYM(SL_IID_ANDROIDSIMPLEBUFFERQUEUE, "ANDROIDSIMPLEBUFFERQUEUE");
    OPENSL_DLSYM(SL_IID_ENGINE, "ENGINE");
    OPENSL_DLSYM(SL_IID_PLAY, "PLAY");
    OPENSL_DLSYM(SL_IID_VOLUME, "VOLUME");
#undef OPENSL_DLSYM

    // create engine
    result = sys->slCreateEnginePtr(&sys->engineObject, 0, NULL, 0, NULL, NULL);
    CHECK_OPENSL_ERROR("Failed to create engine");

    // realize the engine in synchronous mode
    result = Realize(sys->engineObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR("Failed to realize engine");

    // get the engine interface, needed to create other objects
    result = GetInterface(sys->engineObject, sys->SL_IID_ENGINE, &sys->engineEngine);
    CHECK_OPENSL_ERROR("Failed to get the engine interface");

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids1[] = {sys->SL_IID_VOLUME};
    const SLboolean req1[] = {SL_BOOLEAN_FALSE};
    result = CreateOutputMix(sys->engineEngine, &sys->outputMixObject, 1, ids1, req1);
    CHECK_OPENSL_ERROR("Failed to create output mix");

    // realize the output mix in synchronous mode
    result = Realize(sys->outputMixObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR("Failed to realize output mix");

    lock_init(&sys->lock, NULL);

    if (buf_init(&sys->dbt, para->samplerate * 4 / 10) < 0) // 100ms
        goto error;

    aoc->private_data = (void *) sys;
    LOGI("[%s:%d] channels:[%d - %d] samplerate:[%d - %d] \n", __FUNCTION__, __LINE__, para->channels, para->dst_channels, para->samplerate, para->dst_samplerate);
    return 0;

    error:
    if (sys->outputMixObject)
        Destroy(sys->outputMixObject);
    if (sys->engineObject)
        Destroy(sys->engineObject);
    if (sys->p_so_handle)
        dlclose(sys->p_so_handle);
    free(sys);
    LOGI("[%s:%d] open failed. channels:%d samplerate:%d \n", __FUNCTION__, __LINE__, para->dst_channels, para->dst_samplerate);
    return -1;
}

#ifdef ENABLE_DTAP
int dtap_change_effect(ao_wrapper_t *wrapper, int id)
{
    audio_effect_t *ae = (audio_effect_t *)wrapper->ao_priv;
    __android_log_print(ANDROID_LOG_INFO, TAG, "change audio effect from: %d to %d \n", ae->ap.para.item, id);
    lock(&ae->lock);
    ae->ap.para.item = id;
    dtap_update(&ae->ap);
    dtap_init(&ae->ap);
    dt_unlock(&ae->lock);
    return 0;
}
#endif

static int ao_opensl_init(ao_context_t *aoc) {
    if (Open(aoc) == -1)
        return -1;
    Start(aoc);

#ifdef ENABLE_DTAP
    ao_wrapper_t *wrapper = aout->wrapper;
    audio_effect_t *ae = (audio_effect_t *)malloc(sizeof(audio_effect_t));
    wrapper->ao_priv = ae;
    ae->ap.para.samplerate = para->samplerate;
    ae->ap.para.channels = para->channels;
    ae->ap.para.data_width = para->data_width;
    ae->ap.para.type = DTAP_EFFECT_EQ;
    ae->ap.para.item = EQ_EFFECT_NORMAL;
    dtap_init(&ae->ap);
    lock_init(&ae->lock, NULL);
#endif
    return 0;
}

static int ao_opensl_write(ao_context_t *aoc, uint8_t *buf, int size) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    int ret = 0;

    if(!sys) {
        LOGI(TAG, "Opensl empty. not render");
        return 0;
    }
    LOGI(TAG, "[%s:%d] start to write pcm: %d \n", __FUNCTION__, __LINE__, size);
#ifdef ENABLE_DTAP
    audio_effect_t *ae = (audio_effect_t *)wrapper->ao_priv;
    lock(&ae->lock);
    dtap_frame_t frame;
    frame.in = buf;
    frame.in_size = size;
    if(ae->ap.para.item != EQ_EFFECT_NORMAL)
    {
        dtap_process(&ae->ap, &frame);
    }
    dt_unlock(&ae->lock);
#endif

    lock(&sys->lock);
    ret = Play(aoc, buf, size);
    unlock(&sys->lock);
    return ret;
}

static int ao_opensl_pause(ao_context_t *aoc) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    lock(&sys->lock);
    Pause(aoc, 1);
    unlock(&sys->lock);
    return 0;
}

static int ao_opensl_resume(ao_context_t *aoc) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    lock(&sys->lock);
    Pause(aoc, 0);
    unlock(&sys->lock);
    return 0;
}

static int ao_opensl_level(ao_context_t *aoc) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    lock(&sys->lock);
    int level = sys->samples * bytesPerSample(aoc);
    const int unit_size = sys->samples_per_buf * bytesPerSample(aoc);
    SLAndroidSimpleBufferQueueState st;
    SLresult res;
    if (!sys->started)
        goto END;
    res = GetState(sys->playerBufferQueue, &st);
    if ((res != SL_RESULT_SUCCESS)) {
        goto END;
    }
    level += st.count * unit_size;
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "opensl level:%d  st.count:%d sample:%d:%d \n",level, (int)st.count, sys->samples);
    END:
    unlock(&sys->lock);
    return level;
}

static int64_t ao_opensl_get_latency(ao_context_t *aoc) {
    int64_t latency;
    int ret = 0;
    int level = 0;
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;

#if 1
    TimeGet(aoc, &latency);
    if (latency == -1)
        return 0;
    latency = 9 * latency / 100;
#else
    dtaudio_para_t *para = &aout->para;
    level = ao_opensl_level(aout);
    int sample_num;
    float pts_ratio = 0.0;
    pts_ratio = (double) 90000 / para->dst_samplerate;
    sample_num = level / bytesPerSample(aout);
    latency += (sample_num * pts_ratio);
#endif
    //__android_log_print(ANDROID_LOG_DEBUG,TAG, "opensl latency, level:%d latency:%lld \n",level, latency);
    return latency;
}

static int ao_opensl_stop(ao_context_t *aoc) {
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
#ifdef ENABLE_DTAP
    audio_effect_t *ae = (audio_effect_t *)wrapper->ao_priv;
    lock(&ae->lock);
    memset(&ae->ap, 0 , sizeof(dtap_context_t));
    dtap_release(&ae->ap);    
    dt_unlock(&ae->lock);
#endif

    lock(&sys->lock);
    Stop(aoc);
    unlock(&sys->lock);
    return 0;
}

static int ao_opensl_get_volume(ao_context_t *aoc) {
    return 0;
}

static int ao_opensl_set_volume(ao_context_t *aoc, int value) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "opensl setvolume %d \n", value);
    return 0;
}

static int ao_opensl_set_parameter(ao_context_t *aoc, int cmd, unsigned long arg)
{
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    switch (cmd) {
        case DTP_AO_CMD_SET_VOLUME:
            ao_opensl_set_volume(aoc, (int)arg);
            break;
    }
    return 0;
}

static int ao_opensl_get_parameter(ao_context_t *aoc, int cmd, unsigned long arg)
{
    aout_sys_t *sys = (aout_sys_t *) aoc->private_data;
    switch (cmd) {
        case DTP_AO_CMD_GET_LATENCY:
            *(int *)(arg) = ao_opensl_get_latency(aoc);
            break;
        case DTP_AO_CMD_GET_LEVEL:
            *(int *)(arg) = ao_opensl_level(aoc);
            break;
        case DTP_AO_CMD_GET_VOLUME:
            *(int *)(arg) = ao_opensl_get_volume(aoc);
            break;
    }
    return 0;
}

ao_wrapper_t ao_opensl_ops = {
        .id = AO_ID_OPENSL,
        .name = "opensl es",
        .init = ao_opensl_init,
        .pause = ao_opensl_pause,
        .resume = ao_opensl_resume,
        .stop = ao_opensl_stop,
        .write = ao_opensl_write,
        .get_parameter = ao_opensl_get_parameter,
        .set_parameter = ao_opensl_set_parameter,
        .private_data_size = sizeof(aout_sys_t),
};

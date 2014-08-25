/*****************************************************************************
 * opensles_android.c : audio output for android native code
 *****************************************************************************/
/*****************************************************************************
 * Porting From vlc
 *****************************************************************************/

#include "dtaudio_android.h"
#include "dt_buffer.h"
#include "dt_lock.h"

#include <assert.h>
#include <dlfcn.h>
#include <math.h>
#include <stdbool.h>

// For native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define OPENSLES_BUFFERS 255 /* maximum number of buffers */
#define OPENSLES_BUFLEN  10   /* ms */
/*
 * 10ms of precision when mesasuring latency should be enough,
 * with 255 buffers we can buffer 2.55s of audio.
 */

#define CHECK_OPENSL_ERROR(msg)                \
    if (unlikely(result != SL_RESULT_SUCCESS)) \
    {                                          \
        goto error;                            \
    }

typedef SLresult (*slCreateEngine_t)(
        SLObjectItf*, SLuint32, const SLEngineOption*, SLuint32,
        const SLInterfaceID*, const SLboolean*);

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
typedef struct aout_sys_t
{
    /* OpenSL objects */
    SLObjectItf                     engineObject;
    SLObjectItf                     outputMixObject;
    SLAndroidSimpleBufferQueueItf   playerBufferQueue;
    SLObjectItf                     playerObject;
    SLVolumeItf                     volumeItf;
    SLEngineItf                     engineEngine;
    SLPlayItf                       playerPlay;

    /* OpenSL symbols */
    void                           *p_so_handle;

    slCreateEngine_t                slCreateEnginePtr;
    SLInterfaceID                   SL_IID_ENGINE;
    SLInterfaceID                   SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
    SLInterfaceID                   SL_IID_VOLUME;
    SLInterfaceID                   SL_IID_PLAY;

    /* */

    dt_lock_t                     lock;

    /* audio buffered through opensles */
    uint8_t                        *buf;
    size_t                          samples_per_buf;
    int                             next_buf;

    int                             rate;

    /* if we can measure latency already */
    bool                            started;

    /* audio not yet buffered through opensles */
    //block_t                        *p_buffer_chain;
    //block_t                       **pp_buffer_last;

    dt_buffer_t dbt;

    size_t                          samples;
}aout_sys_t;



/*****************************************************************************
 *
 *****************************************************************************/

static inline int bytesPerSample(void)
{
    return 2 /* S16 */ * 2 /* stereo */;
}

typedef int64_t mtime_t;

static int TimeGet(dtaudio_output_t* aout, mtime_t* drift)
{
    aout_sys_t *sys = (aout_sys_t*)aout->ao_priv;

    SLAndroidSimpleBufferQueueState st;
    SLresult res = GetState(sys->playerBufferQueue, &st);
    if (unlikely(res != SL_RESULT_SUCCESS)) {
        return -1;
    }

    dt_lock(&sys->lock);
    bool started = sys->started;
    dt_unlock(&sys->lock);

    if (!started)
        return -1;

    *drift = (CLOCK_FREQ * OPENSLES_BUFLEN * st.count / 1000)
        + sys->samples * CLOCK_FREQ / sys->rate;


    return 0;
}

static void Flush(dtaudio_output_t *aout, bool drain)
{
    aout_sys_t *sys = (aout_sys_t*)aout->ao_priv;

    if (drain) {
        mtime_t delay;
        if (!TimeGet(aout, &delay))
            msleep(delay);
    } else {
        dt_lock(&sys->lock);
        SetPlayState(sys->playerPlay, SL_PLAYSTATE_STOPPED);
        Clear(sys->playerBufferQueue);
        SetPlayState(sys->playerPlay, SL_PLAYSTATE_PLAYING);

        /* release audio data not yet written to opensles */
        //block_ChainRelease(sys->p_buffer_chain);
        //sys->p_buffer_chain = NULL;
        //sys->pp_buffer_last = &sys->p_buffer_chain;

        sys->samples = 0;
        sys->started = false;

        dt_unlock(&sys->lock);
    }
}

#if 0
static int VolumeSet(dtaudio_output_t *aout, float vol)
{
    aout_sys_t *sys = (aout_sys_t)aout->ao_priv;
    if (!sys->volumeItf)
        return -1;

    /* Convert UI volume to linear factor (cube) */
    vol = vol * vol * vol;

    /* millibels from linear amplification */
    int mb = lroundf(2000.f * log10f(vol));
    if (mb < SL_MILLIBEL_MIN)
        mb = SL_MILLIBEL_MIN;
    else if (mb > 0)
        mb = 0; /* maximum supported level could be higher: GetMaxVolumeLevel */

    SLresult r = SetVolumeLevel(aout->sys->volumeItf, mb);
    return (r == SL_RESULT_SUCCESS) ? 0 : -1;
}

static int MuteSet(audio_output_t *aout, bool mute)
{
    if (!aout->sys->volumeItf)
        return -1;

    SLresult r = SetMute(aout->sys->volumeItf, mute);
    return (r == SL_RESULT_SUCCESS) ? 0 : -1;
}
#endif

static void Pause(dtaudio_output_t *aout, bool pause)
{
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;
    SetPlayState(sys->playerPlay,
        pause ? SL_PLAYSTATE_PAUSED : SL_PLAYSTATE_PLAYING);
}

#if 0
static int WriteBuffer(dtaudio_output_t *aout)
{
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;
    const size_t unit_size = sys->samples_per_buf * bytesPerSample();

    block_t *b = sys->p_buffer_chain;
    if (!b)
        return false;

    /* Check if we can fill at least one buffer unit by chaining blocks */
    if (b->i_buffer < unit_size) {
        if (!b->p_next)
            return false;
        ssize_t needed = unit_size - b->i_buffer;
        for (block_t *next = b->p_next; next; next = next->p_next) {
            needed -= next->i_buffer;
            if (needed <= 0)
                break;
        }

        if (needed > 0)
            return false;
    }

    SLAndroidSimpleBufferQueueState st;
    SLresult res = GetState(sys->playerBufferQueue, &st);
    if (unlikely(res != SL_RESULT_SUCCESS)) {
        return false;
    }

    if (st.count == OPENSLES_BUFFERS)
        return false;

    size_t done = 0;
    while (done < unit_size) {
        size_t cur = b->i_buffer;
        if (cur > unit_size - done)
            cur = unit_size - done;

        memcpy(&sys->buf[unit_size * sys->next_buf + done], b->p_buffer, cur);
        b->i_buffer -= cur;
        b->p_buffer += cur;
        done += cur;

        block_t *next = b->p_next;
        if (b->i_buffer == 0) {
            block_Release(b);
            b = NULL;
        }

        if (done == unit_size)
            break;
        else
            b = next;
    }

    sys->p_buffer_chain = b;
    if (!b)
        sys->pp_buffer_last = &sys->p_buffer_chain;

    SLresult r = Enqueue(sys->playerBufferQueue,
        &sys->buf[unit_size * sys->next_buf], unit_size);

    sys->samples -= sys->samples_per_buf;

    if (r == SL_RESULT_SUCCESS) {
        if (++sys->next_buf == OPENSLES_BUFFERS)
            sys->next_buf = 0;
        return true;
    } else {
        /* XXX : if writing fails, we don't retry */
                r, b->i_buffer,
                (r == SL_RESULT_BUFFER_INSUFFICIENT) ? " (buffer insufficient)" : "");
        return false;
    }
}
#endif

static int WriteBuffer(dtaudio_output_t *aout)
{
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;
    const size_t unit_size = sys->samples_per_buf * bytesPerSample();

    /* Check if we can fill at least one buffer unit by chaining blocks */
    if (sys->dbt.level  <  unit_size) {
        return false;
    }

    SLAndroidSimpleBufferQueueState st;
    SLresult res = GetState(sys->playerBufferQueue, &st);
    if (unlikely(res != SL_RESULT_SUCCESS)) {
        return false;
    }

    if (st.count == OPENSLES_BUFFERS)
        return false;

    size_t done = 0;
    while (done < unit_size) {
        size_t cur = buf_level(&sys->dbt);
        if (cur > unit_size - done)
            cur = unit_size - done;

        //memcpy(&sys->buf[unit_size * sys->next_buf + done], b->p_buffer, cur);
        buf_get(&sys->dbt,&sys->buf[unit_size * sys->next_buf + done],cur);
        done += cur;

        if (done == unit_size)
            break;
    }

    SLresult r = Enqueue(sys->playerBufferQueue,
        &sys->buf[unit_size * sys->next_buf], unit_size);

    sys->samples -= sys->samples_per_buf;

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
static int Play(dtaudio_output_t *aout, uint8_t *buf, int size)
{
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;
    dt_lock(&sys->lock);
	int ret = buf_put(&sys->dbt,buf,size);
    sys->samples += ret / bytesPerSample();
    /* Fill OpenSL buffer */
    WriteBuffer(aout);
    dt_unlock(&sys->lock);
    return ret;
}

static void PlayedCallback (SLAndroidSimpleBufferQueueItf caller, void *pContext)
{
    (void)caller;
    dtaudio_output_t *aout = pContext;
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;

    assert (caller == sys->playerBufferQueue);

    dt_lock(&sys->lock);
    sys->started = true;
    dt_unlock(&sys->lock);
}
/*****************************************************************************
 *
 *****************************************************************************/
static int Start(dtaudio_output_t *aout)
{
    SLresult       result;

    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;
    dtaudio_para_t *para = &aout->para;

    // configure audio source - this defines the number of samples you can enqueue.
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        OPENSLES_BUFFERS
    };

    SLDataFormat_PCM format_pcm;
    format_pcm.formatType       = SL_DATAFORMAT_PCM;
    format_pcm.numChannels      = 2;
    format_pcm.samplesPerSec    = ((SLuint32) para->dst_samplerate * 1000) ;
    format_pcm.bitsPerSample    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.containerSize    = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.channelMask      = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    format_pcm.endianness       = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {
        SL_DATALOCATOR_OUTPUTMIX,
        sys->outputMixObject
    };
    SLDataSink audioSnk = {&loc_outmix, NULL};

    //create audio player
    const SLInterfaceID ids2[] = { sys->SL_IID_ANDROIDSIMPLEBUFFERQUEUE, sys->SL_IID_VOLUME };
    static const SLboolean req2[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    result = CreateAudioPlayer(sys->engineEngine, &sys->playerObject, &audioSrc,
                                    &audioSnk, sizeof(ids2) / sizeof(*ids2),
                                    ids2, req2);
    if (unlikely(result != SL_RESULT_SUCCESS)) { // error
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
                                   (void*)aout);
    CHECK_OPENSL_ERROR("Failed to register buff queue callback.");

    // set the player's state to playing
    result = SetPlayState(sys->playerPlay, SL_PLAYSTATE_PLAYING);
    CHECK_OPENSL_ERROR("Failed to switch to playing state");

    /* XXX: rounding shouldn't affect us at normal sampling rate */
    sys->rate = para->dst_samplerate;
    sys->samples_per_buf = OPENSLES_BUFLEN * para->dst_samplerate / 1000;
    sys->buf = malloc(OPENSLES_BUFFERS * sys->samples_per_buf * bytesPerSample());
    if (!sys->buf)
        goto error;

    sys->started = false;
    sys->next_buf = 0;

    //sys->p_buffer_chain = NULL;
    //sys->pp_buffer_last = &sys->p_buffer_chain;
    sys->samples = 0;

    // we want 16bit signed data native endian.
    //fmt->i_format              = VLC_CODEC_S16N;
    //fmt->i_physical_channels   = AOUT_CHAN_LEFT | AOUT_CHAN_RIGHT;

    SetPositionUpdatePeriod(sys->playerPlay, AOUT_MIN_PREPARE_TIME * 1000 / CLOCK_FREQ);

    //aout_FormatPrepare(fmt);

    return 0;

error:
    if (sys->playerObject) {
        Destroy(sys->playerObject);
        sys->playerObject = NULL;
    }

    return -1;
}

static void Stop(dtaudio_output_t *aout)
{
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;

    SetPlayState(sys->playerPlay, SL_PLAYSTATE_STOPPED);
    //Flush remaining buffers if any.
    Clear(sys->playerBufferQueue);

    free(sys->buf);

    Destroy(sys->playerObject);
    sys->playerObject = NULL;
}

/*****************************************************************************
 *
 *****************************************************************************/
static void Close(dtaudio_output_t *aout)
{
    aout_sys_t *sys = (aout_sys_t *)aout->ao_priv;

    Destroy(sys->outputMixObject);
    Destroy(sys->engineObject);
    dlclose(sys->p_so_handle);
    vlc_mutex_destroy(&sys->lock);
    free(sys);
}

static int Open (dtaudio_output_t *aout)
{
    aout_sys_t *sys;
    SLresult result;

    dtaudio_para_t *para = &aout->para;
    sys = calloc(1, sizeof(*sys));
    if (unlikely(sys == NULL))
        return -1;

    sys->p_so_handle = dlopen("libOpenSLES.so", RTLD_NOW);
    if (sys->p_so_handle == NULL)
    {
        goto error;
    }

    sys->slCreateEnginePtr = dlsym(sys->p_so_handle, "slCreateEngine");
    if (unlikely(sys->slCreateEnginePtr == NULL))
    {
        goto error;
    }

#define OPENSL_DLSYM(dest, name)                       \
    do {                                                       \
        const SLInterfaceID *sym = dlsym(sys->p_so_handle, "SL_IID_"name);        \
        if (unlikely(sym == NULL))                             \
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
    const SLInterfaceID ids1[] = { sys->SL_IID_VOLUME };
    const SLboolean req1[] = { SL_BOOLEAN_FALSE };
    result = CreateOutputMix(sys->engineEngine, &sys->outputMixObject, 1, ids1, req1);
    CHECK_OPENSL_ERROR("Failed to create output mix");

    // realize the output mix in synchronous mode
    result = Realize(sys->outputMixObject, SL_BOOLEAN_FALSE);
    CHECK_OPENSL_ERROR("Failed to realize output mix");

    dt_lock_init(&sys->lock, NULL);
	
    if(buf_init(&sys->dbt,para->dst_samplerate * 4 / 10) < 0) // 100ms
        return -1;
    aout->ao_priv = (void *)sys;
#if 0
    aout->start      = Start;
    aout->stop       = Stop;
    aout->time_get   = TimeGet;
    aout->play       = Play;
    aout->pause      = Pause;
    aout->flush      = Flush;
    aout->mute_set   = MuteSet;
    aout->volume_set = VolumeSet;
#endif
    return 0;

error:
    if (sys->outputMixObject)
        Destroy(sys->outputMixObject);
    if (sys->engineObject)
        Destroy(sys->engineObject);
    if (sys->p_so_handle)
        dlclose(sys->p_so_handle);
    free(sys);
    return -1;
}

static int ao_opensl_init (dtaudio_output_t *aout, dtaudio_para_t *para)
{
    Open(aout);
    Start(aout);
}

static int ao_opensl_write (dtaudio_output_t *aout, uint8_t * buf, int size)
{
    return Play(aout,buf,size);
}

static int ao_opensl_pause (dtaudio_output_t *aout)
{
    Pause(aout,1);
    return 0;
}

static int ao_opensl_resume (dtaudio_output_t *aout)
{
    Pause(aout,0);
    return 0;
}

static int ao_opensl_level(dtaudio_output_t *aout)
{

}

static int64_t ao_opensl_get_latency (dtaudio_output_t *aout)
{
    int64_t latency;
    TimeGet(aout, &latency);
    return latency;
}

static int ao_opensl_stop (dtaudio_output_t *aout)
{
    Stop(aout);
}

ao_wrapper_t ao_opensl_ops = {
    .id = AO_ID_OPENSL,
    .name = "opensl es",
    .ao_init = ao_opensl_init,
    .ao_pause = ao_opensl_pause,
    .ao_resume = ao_opensl_resume,
    .ao_stop = ao_opensl_stop,
    .ao_write = ao_opensl_write,
    .ao_level = ao_opensl_level,
    .ao_latency = ao_opensl_get_latency,
};

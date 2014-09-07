// av render wrapper for android

#include "ext_wrapper.h"
#include "dtaudio_android.h"
#include "dtvideo_android.h"

#include <android/log.h>

#define TAG "RENDER_WRAPPER"
/*---------------------------------------------------------- */
extern vo_wrapper_t vo_android_ops;

//vo_wrapper_t vo_ex_ops;

static int vo_ex_init (dtvideo_output_t *vout)
{
    int ret = 0;
    ret = vo_android_ops.vo_init(vout);
    return ret;
}

static int vo_ex_render (dtvideo_output_t *vout,dt_av_pic_t * pict)
{
    int ret = 0;
    ret = vo_android_ops.vo_render(vout,pict);
    return ret;
}

static int vo_ex_stop (dtvideo_output_t *vout)
{
    int ret = 0;
    ret = vo_android_ops.vo_stop(vout);
    return ret;
}

vo_wrapper_t vo_ex_ops = {
    .id = 0x0,//VO_ID_EX,
    .name = "ex vo",
    .vo_init = vo_ex_init,
    .vo_stop = vo_ex_stop,
    .vo_render = vo_ex_render,
};



/*---------------------------------------------------------- */
#ifdef ENABLE_AUDIOTRACK
extern ao_wrapper_t ao_android_ops;
#endif

#ifdef ENABLE_OPENSL
extern ao_wrapper_t ao_opensl_ops;
#endif

static int ao_ex_init (dtaudio_output_t *aout, dtaudio_para_t *para)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_init(aout, para);
#endif

#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_init(aout, para);
#endif
    __android_log_print(ANDROID_LOG_INFO, TAG, "AO Render Init OK");
    return ret;
}

static int ao_ex_play (dtaudio_output_t *aout, uint8_t * buf, int size)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_write(aout, buf,size);
#endif
#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_write(aout, buf,size);
#endif
    return ret;
}

static int ao_ex_pause (dtaudio_output_t *aout)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_pause(aout);
#endif
#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_pause(aout);
#endif
    return ret;
}

static int ao_ex_resume (dtaudio_output_t *aout)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_resume(aout);
#endif
#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_resume(aout);
#endif
    return ret;
}

static int ao_ex_level(dtaudio_output_t *aout)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_level(aout);
#endif
#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_level(aout);
#endif
    return ret;
}

static int64_t ao_ex_get_latency (dtaudio_output_t *aout)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_latency(aout);
#endif
#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_latency(aout);
#endif
    return ret;
}

static int ao_ex_stop (dtaudio_output_t *aout)
{
    int ret = 0;
#ifdef ENABLE_AUDIOTRACK
    ret = ao_android_ops.ao_stop(aout);
#endif
#ifdef ENABLE_OPENSL
    ret = ao_opensl_ops.ao_stop(aout);
#endif
    return ret;
}

ao_wrapper_t ao_ex_ops = {
    .id = 0x0,//AO_ID_EX,
    .name = "ex ao",
    .ao_init = ao_ex_init,
    .ao_pause = ao_ex_pause,
    .ao_resume = ao_ex_resume,
    .ao_stop = ao_ex_stop,
    .ao_write = ao_ex_play,
    .ao_level = ao_ex_level,
    .ao_latency = ao_ex_get_latency,
};

extern void ao_audiotrack_init(); // setup android audio render
extern void vd_stagefright_init(); // setup android audio render
extern vd_wrapper_t vd_stagefright_ops;

int ext_element_init()
{
#ifdef ENABLE_ANDROID_OMX
    vd_stagefright_init();
    dtplayer_register_ext_vd(&vd_stagefright_ops);
#endif
#ifdef ENABLE_AUDIOTRACK
	ao_audiotrack_init();
#endif
    dtplayer_register_ext_ao(&ao_ex_ops);
    dtplayer_register_ext_vo(&vo_ex_ops);
    __android_log_print(ANDROID_LOG_INFO, TAG, "EXT Element Init OK");
    return  0;
}

int ext_element_stop()
{
	__android_log_print(ANDROID_LOG_INFO, TAG, "EXT Element Stop OK");
    return 0;
}

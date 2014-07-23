// av render wrapper for android

#include "render_android.h"
#include "dtaudio_android.h"
#include "dtvideo_android.h"

#include <android/log.h>

#define TAG "RENDER_WRAPPER"
/*---------------------------------------------------------- */
extern vo_wrapper_t vo_android_ops;

//vo_wrapper_t vo_ex_ops;

static int vo_ex_init ()
{
    int ret = 0;
    ret = vo_android_ops.vo_init();
    return ret;
}

static int vo_ex_render (AVPicture_t * pict)
{
    int ret = 0;
    ret = vo_android_ops.vo_render(pict);
    return ret;
}

static int vo_ex_stop ()
{
    int ret = 0;
    ret = vo_android_ops.vo_stop();
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
extern ao_wrapper_t ao_android_ops;

static int ao_ex_init (dtaudio_para_t *para)
{
    int ret = 0;
    ret = ao_android_ops.ao_init(para);
    __android_log_print(ANDROID_LOG_INFO, TAG, "AO Render Init OK");
    return ret;
}

static int ao_ex_play (uint8_t * buf, int size)
{
    int ret = 0;
    ret = ao_android_ops.ao_write(buf,size);
    return ret;
}

static int ao_ex_pause ()
{
    int ret = 0;
    ret = ao_android_ops.ao_pause();
    return ret;
}

static int ao_ex_resume ()
{
    int ret = 0;
    ret = ao_android_ops.ao_resume();
    return ret;
}

static int ao_ex_level()
{
    int ret = 0;
    ret = ao_android_ops.ao_level();
    return ret;
}

static int64_t ao_ex_get_latency ()
{
    int ret = 0;
    ret = ao_android_ops.ao_latency();
    return ret;
}

static int ao_ex_stop ()
{
    int ret = 0;
    ret = ao_android_ops.ao_stop();
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

extern void android_ao_init(); // setup android audio render

int render_init()
{
	android_ao_init();
    register_ext_ao(&ao_ex_ops);
    register_ext_vo(&vo_ex_ops);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Render Init OK");
    return  0;
}

int render_stop()
{
	__android_log_print(ANDROID_LOG_INFO, TAG, "Render Stop OK");
    return 0;
}

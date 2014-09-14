// android audio effect plugin

#include "ae_wrapper.h"

static ae_wrapper_t ae_android_ops;

int android_ae_capacity()
{
    return ae_android_ops.capacity();
}

int android_ae_init(int ae_id)
{
    return ae_android_ops.init(&ae_android_ops, ae_id)
}

int android_ae_process(dt_av_frame_t *frame)
{
    return ae_android_ops.process(&ae_android_ops, frame);
}

int android_reset_ae(int ae_id)
{
    return ae_android_ops.reset(ae_id);
}

int android_ae_release()
{
    return ae_android_ops.release();
}

static int ae_android_capacity()
{
    int capacity = 0;
    capacity |= DT_AE_CLASSICAL;
    capacity |= DT_AE_DANCE;
    capacity |= DT_AE_FLAT;
    capacity |= DT_AE_FOLK;
    capacity |= DT_AE_HEAVY_METAL;
    capacity |= DT_AE_HIPPOP;
    capacity |= DT_AE_JAZZ;
    return capacity;
}

static int ae_android_init(dtaudio_effect_t *ae ,int ae_id)
{
    return 0; 
}

static int ae_android_reset(dtaudio_effect_t *ae ,int ae_id)
{
    return 0; 
}

static int ae_android_process(dtaudio_effect_t *ae, dt_av_frame_t *frame)
{
    return 0;
}

static int ae_android_release(dtaudio_effect_t *ae)
{

}

static ae_wrapper_t ae_android_ops = {
    .id = 0x0,
    .name = "ae android",
    .capacity = ae_android_capacity,
    .init = ae_android_init,
    .reset = ae_android_reset,
    .process = ae_android_process,
    .release = ae_android_release,
};

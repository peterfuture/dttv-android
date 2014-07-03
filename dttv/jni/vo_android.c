//render image with android

#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdint.h>

#include "dtvideo_android.h"
#include "dt_lock.h"

#define TAG "VO-ANDROID"

vo_wrapper_t vo_android_ops;

typedef struct{
	uint8_t *buf; //buffer provider for surfaceview
    dt_lock_t vo_mutex;
    int dx,dy,dw,dh;
    int sdl_inited;
}vo_android_ctx_t;

static vo_android_ctx_t vo_android_ctx;

int update_frame(uint8_t *buf,int size);

static int vo_android_init ()
{
    vo_wrapper_t *wrap = &vo_android_ops;
    wrap->handle = (void *)&vo_android_ctx;
    __android_log_print(ANDROID_LOG_DEBUG,TAG, "android vo init OK\n");
    return 0;
}

static int vo_android_render (AVPicture_t * pict)
{
    vo_wrapper_t *wrap = &vo_android_ops;
    vo_android_ctx_t *ctx = (vo_android_ctx_t *)wrap->handle;
    
    dt_lock (&ctx->vo_mutex);
    update_frame(pict->data[0],ctx->dw*ctx->dh*2);
    dt_unlock (&ctx->vo_mutex);
    return 0;
}

static int vo_android_stop ()
{
    vo_wrapper_t *wrap = &vo_android_ops;
    vo_android_ctx_t *ctx = (vo_android_ctx_t *)wrap->handle;
    wrap->handle = NULL;
    __android_log_print(ANDROID_LOG_DEBUG,TAG,"stop vo sdl\n");
    return 0;
}

vo_wrapper_t vo_android_ops = {
    .id = 0x100,//VO_ID_ANDROID,
    .name = "vo android",
    .vo_init = vo_android_init,
    .vo_stop = vo_android_stop,
    .vo_render = vo_android_render,
};

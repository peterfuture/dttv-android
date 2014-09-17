//render image with android

#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <stdint.h>

#include "dtvideo_android.h"
#include "dt_lock.h"

#define TAG "VO-ANDROID"

vo_wrapper_t vo_android_ops;

typedef struct{
    dt_lock_t vo_mutex;
    int dx,dy,dw,dh;
    int sdl_inited;
}vo_android_ctx_t;

static vo_android_ctx_t vo_android_ctx;

int update_frame(uint8_t *buf,int size);

static void SaveFrame (dt_av_frame_t * pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[32];
    int y;

    // Open file
    sprintf (szFilename, "/data/frame%d.ppm", iFrame);
    pFile = fopen (szFilename, "wb");
    if (pFile == NULL)
        return;

    // Write header
    fprintf (pFile, "P6\n%d %d\n255\n", width, height);

    
    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite (pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    
    // Close file
    fclose (pFile);
}

static int vo_android_init (dtvideo_output_t *vout)
{
    vo_wrapper_t *wrap = &vo_android_ops;
    wrap->handle = (void *)&vo_android_ctx;
    vo_android_ctx.dx = 0;
    vo_android_ctx.dy = 0;
    //get w h from activity
    //vo_android_ctx.dw = getActivityWidth();
    //vo_android_ctx.dh = getActivityHeight();
    
    vo_android_ctx.dw = vout->para->d_width;
    vo_android_ctx.dh = vout->para->d_height;

    __android_log_print(ANDROID_LOG_DEBUG,TAG, "android vo init OK, width:%d height:%d \n",vo_android_ctx.dw, vo_android_ctx.dh);

    return 0;
}

static int vo_android_render (dtvideo_output_t *vout,dt_av_frame_t * pict)
{
    vo_wrapper_t *wrap = &vo_android_ops;
    vo_android_ctx_t *ctx = (vo_android_ctx_t *)wrap->handle;
    
    dt_lock (&ctx->vo_mutex);
    update_frame(pict->data[0],ctx->dw*ctx->dh*2);
    pict->data[0] = NULL;
    __android_log_print(ANDROID_LOG_DEBUG,TAG,"render one frame ok\n");
    dt_unlock (&ctx->vo_mutex);
    return 0;
}

static int vo_android_stop (dtvideo_output_t *vout)
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

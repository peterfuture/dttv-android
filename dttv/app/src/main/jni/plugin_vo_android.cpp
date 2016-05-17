#include "native_log.h"
#define TAG "VO-ANDROID"

#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <stdint.h>

#include "dtvideo_android.h"
#include "../../../../3rd/libdtp/include/vo_wrapper.h"
#include "dt_lock.h"
#include "gl_yuv.h"

static int vo_android_init (dtvideo_output_t *vout);
static int vo_android_render (dtvideo_output_t *vout,dt_av_frame_t * frame);
static int vo_android_stop (dtvideo_output_t *vout);

struct vo_info{
    int dx,dy,dw,dh;
    dt_lock_t mutex;
};

vo_wrapper_t vo_android = {
        .id = 0x100,//VO_ID_ANDROID,
        .name = "vo android",
        .vo_init = vo_android_init,
        .vo_stop = vo_android_stop,
        .vo_render = vo_android_render,
};

static void dump_frame (dt_av_frame_t * pFrame, int width, int height, int iFrame)
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
    struct vo_info *info = (struct vo_info *)malloc(sizeof(struct vo_info));
    vo_android.handle = info;
    info->dx = 0;
    info->dy = 0;
    info->dw = vout->para->d_width;
    info->dh = vout->para->d_height;
    LOGV("android vo init OK, w:%d h:%d\n",info->dw, info->dh);
    return 0;
}

static int vo_android_render (dtvideo_output_t *vout,dt_av_frame_t * frame)
{
    struct vo_info *info = (struct vo_info *)vo_android.handle;
    int size = info->dw * info->dh * 1.5; // yuv 420 size

    dt_lock (&info->mutex);
    yuv_update_frame(frame);
    frame->data[0] = NULL;
    //LOGV("render one frame ok\n");
    dt_unlock (&info->mutex);
    return 0;
}

static int vo_android_stop (dtvideo_output_t *vout)
{
    struct vo_info *info = (struct vo_info *)vo_android.handle;
    free(info);
    vo_android.handle = NULL;
    LOGV("stop vo android\n");
    return 0;
}

void vo_android_setup(vo_wrapper_t **vo)
{
    *vo = &vo_android;
    return ;
}
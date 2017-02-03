#define TAG "VO-ANDROID"

#include <string.h>
#include <android/log.h>

#include "dtp_av.h"
#include "dtp_plugin.h"
#include "dtp_vf.h"
#include "gl_yuv.h"
#include "../dttv_jni_utils.h"
#include "../dttv_jni_log.h"


static int vo_android_init(vo_wrapper_t *vout);
static int vo_android_render(vo_wrapper_t *vout, dt_av_frame_t *frame);
static int vo_android_stop(vo_wrapper_t *vout);

struct vo_info {
    int dx, dy, dw, dh;
    lock_t mutex;
};

vo_wrapper_t vo_android = {
        .id = 0x100,//VO_ID_ANDROID,
        .name = "vo android",
        .init = vo_android_init,
        .stop = vo_android_stop,
        .render = vo_android_render,
};

static dtvideo_filter_t glvf;

static void dump_frame(dt_av_frame_t *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int y;
    // Open file
    sprintf(szFilename, "/data/frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL) {
        return;
    }
    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    // Write pixel data
    for (y = 0; y < height; y++) {
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    }
    // Close file
    fclose(pFile);
}

static int vo_android_init(vo_wrapper_t *vout) {
    struct vo_info *info = (struct vo_info *) malloc(sizeof(struct vo_info));
    vo_android.handle = info;
    info->dx = 0;
    info->dy = 0;
    info->dw = vout->para.d_width;
    info->dh = vout->para.d_height;
    memset(&glvf, 0, sizeof(dtvideo_filter_t));
    memcpy(&glvf.para, &vout->para, sizeof(dtvideo_para_t));
    lock_init(&info->mutex, NULL);
    LOGV("android vo init OK, w:%d h:%d\n", info->dw, info->dh);
    return 0;
}

static int vo_android_render(vo_wrapper_t *vout, dt_av_frame_t *frame) {
    struct vo_info *info = (struct vo_info *) vo_android.handle;

    // reset vf and window size
    dtvideo_filter_t *vf = &glvf;
    if (frame->pixfmt != DTAV_PIX_FMT_YUV420P) {
        vf->para.s_width = frame->width;
        vf->para.s_height = frame->height;
        vf->para.d_width = info->dw;
        vf->para.d_height = info->dh;
        vf->para.s_pixfmt = frame->pixfmt;
        vf->para.d_pixfmt = DTAV_PIX_FMT_YUV420P;
        video_filter_update(vf);
        LOGV("Need to Update Video Filter Parameter.\n");
    }

    video_filter_process(vf, frame);
    int size = info->dw * info->dh * 3 / 2; // yuv 420 size
    lock(&info->mutex);
    yuv_update_frame(frame);
    frame->data[0] = NULL;
    unlock(&info->mutex);
    return 0;
}

static int vo_android_stop(vo_wrapper_t *vout) {
    struct vo_info *info = (struct vo_info *) vo_android.handle;
    free(info);
    vo_android.handle = NULL;
    video_filter_stop(&glvf);
    LOGV("stop vo android\n");
    return 0;
}

void vo_android_setup(vo_wrapper_t *vo) {
    *vo = vo_android;
    return;
}
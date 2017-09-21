#define TAG "VO-ANDROID"

#include <string.h>
#include <android/log.h>
#include <dtp_av.h>

#include "dtp_plugin.h"
#include "dtp_vf.h"
#include "gl_render.h"
#include "dttv_jni_utils.h"
#include "dttv_jni_log.h"

struct vo_info {
    lock_t mutex;
};

static dtvideo_filter_t glvf;

static int pix_fmt = -1;

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

static int vo_android_init(vo_context_t *voc) {
    struct vo_info *info = (struct vo_info *) voc->private_data;
    lock_init(&info->mutex, NULL);
    memset(&glvf, 0, sizeof(dtvideo_filter_t));
    LOGV("android vo init OK");
    return 0;
}

static int vo_android_render(vo_context_t *voc, dt_av_frame_t *frame) {
    struct vo_info *info = (struct vo_info *) voc->private_data;

    if(pix_fmt == -1) {
        int ret = gl_get_parameter(KEY_PARAMETER_GLRENDER_GET_PIXFMT, (unsigned long) (&pix_fmt));
        if (ret < 0) {
            dtp_frame_free(frame, 0);
            return 0;
        }
    }
    // reset vf and window size
    dtvideo_filter_t *vf = &glvf;
    if (vf->para.d_width != frame->width
        || vf->para.d_height != frame->height
        || vf->para.d_pixfmt != pix_fmt) {
        vf->para.d_width = frame->height;
        vf->para.d_height = frame->height;
        vf->para.s_width = frame->width;
        vf->para.s_height = frame->height;
        vf->para.s_pixfmt = frame->pixfmt;
        vf->para.d_pixfmt = pix_fmt;
        LOGV("Need to Update Video Filter Parameter. w:%d->%d h:%d->%d pixfmt:%d->%d \n",
             vf->para.s_width, vf->para.d_width, vf->para.s_height, vf->para.d_height,
             vf->para.s_pixfmt, vf->para.d_pixfmt);
        video_filter_update(vf);
    }
    video_filter_process(vf, frame);
    gl_update_frame(frame);
    frame->data[0] = NULL;
    return 0;
}

static int vo_android_stop(vo_context_t *voc) {
    struct vo_info *info = (struct vo_info *) voc->private_data;
    video_filter_stop(&glvf);
    pix_fmt = -1;
    LOGV("stop vo android\n");
    return 0;
}

vo_wrapper_t vo_android_opengl = {
    .id = 0x100,//VO_ID_ANDROID,
    .name = "vo android",
    .init = vo_android_init,
    .stop = vo_android_stop,
    .render = vo_android_render,
    .private_data_size = sizeof(struct vo_info),
};

#define TAG "VO-ANDROID-SURFACE"

#include <android/native_window_jni.h>
#include <dtp_vf.h>
#include <dtp_av.h>
#include "dttv_jni_log.h"

struct surface_context {
    int dx, dy, dw, dh;
    ANativeWindow *window;
    int native_window_init;
};

static dtvideo_filter_t vf_surface;

static int vo_surface_init(vo_wrapper_t *vout) {

    struct surface_context *context = (struct surface_context *) malloc(
            sizeof(struct surface_context));
    vout->handle = context;
    context->dx = 0;
    context->dy = 0;
    context->dw = vout->para.d_width;
    context->dh = vout->para.d_height;

    if(vout->vo_priv) {
        context->window = (ANativeWindow *) vout->vo_priv;
        LOGV("VOUT Window Addr:%p \n", context->window);
        ANativeWindow_setBuffersGeometry(context->window, context->dw, context->dh,
                                         WINDOW_FORMAT_RGBA_8888);
        context->native_window_init = 1;
    } else {
        context->window = NULL;
        context->native_window_init = 0;
    }
    LOGV("android vo init OK, w:%d h:%d\n", context->dw, context->dh);

    return 0;
}

static int vo_surface_render(vo_wrapper_t *vout, dt_av_frame_t *frame) {

    struct surface_context *context = (struct surface_context *) vout->handle;

    if(vout->vo_priv == NULL) {
        LOGV("android vo surface not set\n");
        return 0;
    }

    if(context->native_window_init == 0) {
        context->window = (ANativeWindow *) vout->vo_priv;
        ANativeWindow_setBuffersGeometry(context->window, context->dw, context->dh,
                                         WINDOW_FORMAT_RGBA_8888);
        context->native_window_init = 1;
        LOGV("android vo surface set ok. width:%d\n", ANativeWindow_getWidth(context->window));
    }

    // filter
    dtvideo_filter_t *vf = &vf_surface;
    if (frame->pixfmt != DTAV_PIX_FMT_RGBA) {
        vf->para.s_width = frame->width;
        vf->para.s_height = frame->height;
        vf->para.d_width = context->dw;
        vf->para.d_height = context->dh;
        vf->para.s_pixfmt = frame->pixfmt;
        vf->para.d_pixfmt = DTAV_PIX_FMT_RGBA;
        video_filter_update(vf);
        LOGV("Need to Update Video Filter Parameter.\n");
    }
    video_filter_process(vf, frame);

    ANativeWindow_Buffer buf;
    ANativeWindow *window = context->window;
    if (ANativeWindow_lock(window, &buf, NULL) == 0) {
        LOGV("Render one frame. dst:%p src:%p size:%d.\n", buf.bits, frame->data[0], frame->linesize[0]);
        if (buf.width >= buf.stride)
            memcpy(buf.bits, frame->data[0], context->dw*context->dh*4);
#if 0
        // 获取stride
        uint8_t * dst = (uint8_t*)buf.bits;
        int dstStride = buf.stride * 4;
        uint8_t * src = (uint8_t*) (frame->data[0]);
        int srcStride = frame->linesize[0];

        // 由于window的stride和帧的stride不同,因此需要逐行复制
        int h;
        for (h = 0; h < context->dh; h++) {
            memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
        }
#endif
        ANativeWindow_unlockAndPost(window);

    } else {
        LOGV("Render one frame failed.\n");
        return 0;
    }
    LOGV("Render one frame ok.\n");
    return 0;

}

static int vo_surface_stop(vo_wrapper_t *vout) {
    struct surface_context *context = (struct surface_context *) vout->handle;
    video_filter_stop(&vf_surface);
    if(context->window) {
        ANativeWindow *window = context->window;
        ANativeWindow_release(window);
    }
    free(context);
    vout->vo_priv = NULL;

    return 0;
}

vo_wrapper_t vo_android_surface = {
        .id = 0x101,//VO_ID_ANDROID_SURFACE,
        .name = "vo surface",
        .init = vo_surface_init,
        .stop = vo_surface_stop,
        .render = vo_surface_render,
};
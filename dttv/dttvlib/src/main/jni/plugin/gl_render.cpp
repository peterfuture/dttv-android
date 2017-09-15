#include <dtp_av.h>

#include <gl_render.h>
#include <gl_render_yuv.h>
#include <gl_render_rgb.h>
#include <dttv_jni_dtp.h>

struct gl_ctx {
    int width;
    int height;
    int pixfmt;

    lock_t mutex;
    void *handle;
};

static gl_ctx ctx;

using namespace android;

int gl_create(void *h, int pixfmt)
{
    ctx = {0};
    ctx.handle = h;
    ctx.pixfmt = pixfmt;
    lock_init(&ctx.mutex, NULL);
    yuv_dttv_init();
    return 0;
}

int gl_setup(int w, int h)
{
    ctx.width = w;
    ctx.height = h;
    yuv_setupGraphics(w, h);
    return 0;
}

int gl_notify()
{
    if(ctx.handle == 0) {
        return 0;
    }
    static android::DTPlayer * player = (android::DTPlayer *) ctx.handle;
    player->Notify(MEDIA_FRESH_VIDEO);
    return 0;
}

int gl_update_frame(dt_av_frame_t *frame)
{
    if(ctx.handle == 0) {
        return 0;
    }
    yuv_update_frame(frame);
    return 0;
}

int gl_render()
{
    yuv_renderFrame();
    return 0;
}
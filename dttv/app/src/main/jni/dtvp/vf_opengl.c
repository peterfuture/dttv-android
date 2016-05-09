/***********************************************************************
**
**  Module : vf_opengl.c
**  Summary: video post processor with opengl
**  Section: dtvideo
**  Author : peter
**  Notes  : 
**           clip & colorspace convert for Android
**
***********************************************************************/

#include "vf_wrapper.h"
#define TAG "VF-FFMPEG"
typedef struct vf_opengl_ctx
{
    int need_process;
    dt_av_frame_t *swap_frame;
    int swap_buf_size;
    uint8_t *swapbuf;
}vf_opengl_ctx_t;

/***********************************************************************
**
** capability check
**
***********************************************************************/
static int opengl_vf_capable(vf_cap_t cap)
{
    int opengl_cap = VF_CAP_COLORSPACE_CONVERT | VF_CAP_CLIP;
    dt_debug(TAG, "request cap: %x , %s support:%x \n", cap, "opengl vf", ffmpeg_cap);
    return cap & ffmpeg_cap; 
}

vf_wrapper_t vf_opengl_ops = {
    .name       = "opengl video filter",
    .type       = DT_TYPE_VIDEO,
    .capable    = opengl_vf_capable,
    .init       = ffmpeg_vf_init,
    .process    = ffmpeg_vf_process,
    .release    = ffmpeg_vf_release,
};

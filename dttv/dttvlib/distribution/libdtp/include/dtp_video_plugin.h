#ifndef DTP_VIDEO_PLUGIN_H
#define DTP_VIDEO_PLUGIN_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>

#include "dtp_av.h"

/* video render plugin */

#define VIDEO_EXTRADATA_SIZE   4096

typedef struct {
    int vfmt;
    int d_width;                //dest w
    int d_height;               //dest h
    int s_width;                //src w
    int s_height;               //src h
    int d_pixfmt;               //dest pixel format
    int s_pixfmt;               //src pixel format
    int rate;
    int ratio;
    double fps;
    int num, den;               //for pts calc
    int extradata_size;
    unsigned char extradata[VIDEO_EXTRADATA_SIZE];
    int flag;                   // setting from player
    int video_filter;
    int video_output;
    void *avctx_priv;
} dtvideo_para_t;

struct vo_wrapper;

typedef int (*vo_init)(struct vo_wrapper *vo);
typedef int (*vo_stop)(struct vo_wrapper *vo);
typedef int (*vo_render)(struct vo_wrapper *vo, dt_av_frame_t * pic);

typedef struct vo_wrapper {
    int id;
    const char *name;
    dtvideo_para_t para;

    vo_init init;
    vo_render render;
    vo_stop stop;

    void *handle;
    struct vo_wrapper *next;
    void *vo_priv;
} vo_wrapper_t;

/* video filter plugin */

typedef enum {
    VF_CAP_NOP                = 0x0,
    VF_CAP_COLORSPACE_CONVERT = 0x1,
    VF_CAP_CLIP               = 0x2,
    VF_CAP_MAX                = 0x80000
} vf_cap_t;

struct vf_wrapper;

typedef int (*vf_init)(struct vf_wrapper *vf);
typedef int (*vf_process)(struct vf_wrapper *vf, dt_av_frame_t *frame);
typedef int (*vf_capable)(vf_cap_t cap);
typedef int (*vf_release)(struct vf_wrapper *vf);

typedef struct vf_wrapper {
    dtvideo_para_t para;
    char *name;
    int type;

    vf_init init;
    vf_process process;
    vf_capable capable;
    vf_release release;

    struct vf_wrapper *next;
    void *vf_priv;
} vf_wrapper_t;

#ifdef  __cplusplus
}
#endif

#endif

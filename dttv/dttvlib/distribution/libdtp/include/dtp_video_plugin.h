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
    void *device;               // video render device. Ext: surfaceview for android
    void *avctx_priv;
} dtvideo_para_t;

struct vo_wrapper;
struct vo_context;

typedef int (*vo_init)(struct vo_context *ctx);
typedef int (*vo_render)(struct vo_context *ctx, dt_av_frame_t * frame);
typedef int (*vo_stop)(struct vo_context *ctx);

typedef struct vo_wrapper {
    int id;
    const char *name;

    vo_init init;
    vo_render render;
    vo_stop stop;

    struct vo_wrapper *next;
    int private_data_size;
} vo_wrapper_t;

/* video filter plugin */

typedef enum {
    VF_CAP_NOP                = 0x0,
    VF_CAP_COLORSPACE_CONVERT = 0x1,
    VF_CAP_CLIP               = 0x2,
    VF_CAP_MAX                = 0x80000
} vf_cap_t;

struct vf_wrapper;
struct vf_context;

typedef int (*vf_init)(struct vf_context *vfc);
typedef int (*vf_process)(struct vf_context *vfc, dt_av_frame_t *frame);
typedef int (*vf_capable)(vf_cap_t cap);
typedef int (*vf_release)(struct vf_context *vfc);

typedef struct vf_wrapper {
    char *name;
    int type;

    vf_init init;
    vf_process process;
    vf_capable capable;
    vf_release release;

    struct vf_wrapper *next;
    int private_data_size;
} vf_wrapper_t;

typedef struct vo_context {
    dtvideo_para_t para;
    vo_wrapper_t *wrapper;
    void *private_data;
} vo_context_t;

typedef struct vf_context {
    dtvideo_para_t para;
    vf_wrapper_t *wrapper;
    void *private_data;
} vf_context_t;

#ifdef  __cplusplus
}
#endif

#endif

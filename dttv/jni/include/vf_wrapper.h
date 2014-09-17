#ifndef VF_FILTER_H
#define VF_FILTER_H

#include "dt_av.h"
#include "dtvideo_para.h"

struct dtvideo_filter;

typedef enum{
    VF_CAP_NOP     = 0x0,
    VF_CAP_CONVERT = 0x1,
    VF_CAP_CROP    = 0x2,
    VF_CAP_MAX     = 0x80000
}vf_cap_t;

typedef enum{
    VF_STATUS_ERROR   = -1,
    VF_STATUS_IDLE    = 0,
    VF_STATUS_RUNNING = 1
}vf_status_t;

typedef struct vf_wrapper
{
    char *name;
    int type;
    int (*capable) (struct dtvideo_filter *vf);
    int (*init) (struct dtvideo_filter *vf);
    int (*process) (struct dtvideo_filter *vf, dt_av_frame_t *pic);
    int (*release) (struct dtvideo_filter *vf);
    struct vf_wrapper *next;
} vf_wrapper_t;

typedef struct dtvideo_filter
{
    dtvideo_para_t *para;
    vf_wrapper_t *wrapper;
    dt_lock_t mutex;
    int status;
    void *vf_priv;
    void *parent;
}dtvideo_filter_t;

#endif

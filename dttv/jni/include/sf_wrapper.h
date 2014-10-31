#ifndef SF_FILTER_H
#define SF_FILTER_H

#include "dt_av.h"
#include "dtsub_para.h"

struct dtsub_filter;

typedef enum{
    SF_CAP_NOP                = 0x0,
    SF_CAP_MAX                = 0x80000
}sf_cap_t;

typedef enum{
    SF_STATUS_ERROR   = -1,
    SF_STATUS_IDLE    = 0,
    SF_STATUS_RUNNING = 1
}sf_status_t;

typedef struct sf_wrapper
{
    char *name;
    int type;
    int (*capable) (sf_cap_t cap);
    int (*init) (struct dtsub_filter *sf);
    int (*process) (struct dtsub_filter *sf, dt_av_frame_t *frame);
    int (*release) (struct dtsub_filter *sf);
    struct sf_wrapper *next;
}sf_wrapper_t;

typedef struct dtsub_filter
{
    dtsub_para_t para;
    sf_wrapper_t *wrapper;
    dt_lock_t mutex;
    int status;
    void *sf_priv;
    void *parent;
}dtsub_filter_t;

#endif

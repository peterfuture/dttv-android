#ifndef DTP_VF_H
#define DTP_VF_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "dtp_video_plugin.h"

typedef enum {
    VF_STATUS_ERROR   = -1,
    VF_STATUS_IDLE    = 0,
    VF_STATUS_RUNNING = 1
}
vf_status_t;

typedef struct dtvideo_filter {
    dtvideo_para_t para;
    vf_context_t *vfc;
    int status;
    void *vf_priv;
    void *parent;
} dtvideo_filter_t;

void vf_register_all();
void vf_remove_all();
void vf_register_ext(vf_wrapper_t *vf);

int video_filter_init(dtvideo_filter_t * filter);
int video_filter_update(dtvideo_filter_t * filter);
int video_filter_process(dtvideo_filter_t * filter, dt_av_frame_t *frame);
int video_filter_stop(dtvideo_filter_t * filter);

#ifdef  __cplusplus
}
#endif

#endif

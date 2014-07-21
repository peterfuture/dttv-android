#ifndef VD_WRAPPER_H
#define VD_WRAPPER_H

#include "dt_buffer.h"
#include "dt_av.h"
#include "dtvideo_para.h"

typedef enum
{
    VDEC_STATUS_IDLE,
    VDEC_STATUS_RUNNING,
    VDEC_STATUS_PAUSED,
    VDEC_STATUS_EXIT
} vdec_status_t;

typedef struct vd_wrapper
{
    char *name;
    video_format_t vfmt;        // not used, for ffmpeg
    int type;

    int (*init) (struct vd_wrapper *wrapper, void *parent);
    int (*decode_frame) (struct vd_wrapper *wrapper, dt_av_frame_t * frame, AVPicture_t ** pic);
    int (*release) (struct vd_wrapper *wrapper);
    
    void *vd_priv;
    struct vd_wrapper *next;
    void *parent;
} vd_wrapper_t;

typedef struct dtvideo_decoder
{
    dtvideo_para_t para;
    vd_wrapper_t *wrapper;
    pthread_t video_decoder_pid;
    vdec_status_t status;
    int decode_err_cnt;

    int64_t pts_current;
    int64_t pts_first;
    unsigned int pts_last_valid;
    unsigned int pts_buffer_size;
    unsigned int pts_cache_size;
    int frame_count;

    dt_buffer_t *buf_out;
    void *parent;
    void *decoder_priv;
}dtvideo_decoder_t;

#endif

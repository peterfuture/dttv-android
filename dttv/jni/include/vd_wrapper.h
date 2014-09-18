#ifndef VD_WRAPPER_H
#define VD_WRAPPER_H

#include "dt_buffer.h"
#include "dt_av.h"
#include "dtvideo_para.h"

struct dtvideo_decoder;

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
    dtvideo_format_t vfmt;        // not used, for ffmpeg
    dtvideo_para_t *para;          // info changed needed 
    int type;
    int is_hw;

    int (*init) (struct dtvideo_decoder *decoder);
    int (*decode_frame) (struct dtvideo_decoder *decoder, dt_av_pkt_t * frame, dt_av_frame_t ** pic);
    int (*info_changed) (struct dtvideo_decoder *decoder);
    int (*release) (struct dtvideo_decoder *decoder);
    
    void *vd_priv;
    struct vd_wrapper *next;
    void *parent;
} vd_wrapper_t;

typedef struct dtvideo_decoder
{
    dtvideo_para_t *para;
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
    void *vd_priv;
}dtvideo_decoder_t;

#endif

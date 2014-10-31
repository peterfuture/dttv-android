#ifndef SD_WRAPPER_H
#define SD_WRAPPER_H

#include "dt_buffer.h"
#include "dt_av.h"
#include "dtsub_para.h"

struct dtsub_decoder;

typedef enum
{
    SDEC_STATUS_IDLE,
    SDEC_STATUS_RUNNING,
    SDEC_STATUS_PAUSED,
    SDEC_STATUS_EXIT
}sdec_status_t;

typedef struct sd_wrapper
{
    const char *name;
    dtsub_para_t *para;
    dtsub_format_t sfmt;
    int type;

    int (*init) (struct dtsub_decoder *decoder);
    int (*decode_frame) (struct dtsub_decoder *decoder, dt_av_pkt_t * pkt, dtav_sub_frame_t ** frame);
    int (*release) (struct dtsub_decoder *decoder);
    
    void *sd_priv;
    struct sd_wrapper *next;
    void *parent;
}sd_wrapper_t;

typedef struct dtsub_decoder
{
    dtsub_para_t *para;
    sd_wrapper_t *wrapper;
    pthread_t sub_decoder_pid;
    sdec_status_t status;
    int decode_err_cnt;

    int64_t pts_current;
    int64_t pts_first;
    unsigned int pts_last_valid;
    int frame_count;

    void *parent;
    void *sd_priv;
}dtsub_decoder_t;

#endif

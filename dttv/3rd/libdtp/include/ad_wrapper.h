#ifndef AD_WRAPPER_H
#define AD_WRAPPER_H

#include "dt_buffer.h"
#include "dt_av.h"
#include "dtaudio_para.h"

typedef enum {
    ADEC_STATUS_IDLE,
    ADEC_STATUS_RUNNING,
    ADEC_STATUS_PAUSED,
    ADEC_STATUS_EXIT
} adec_status_t;

typedef struct {
    uint8_t *inptr;
    int inlen;
    int consume;
    uint8_t *outptr;
    int outsize; // buffer size
    int outlen;  // buffer level

    int info_change;
    int channels;
    int samplerate;
    int bps;
} adec_ctrl_t;

typedef struct ad_wrapper {
    int (*init)(struct ad_wrapper * wrapper, void *parent);
    int (*decode_frame)(struct ad_wrapper * wrapper, adec_ctrl_t *pinfo);
    int (*release)(struct ad_wrapper * wrapper);
    char *name;
    dtaudio_format_t afmt;        //not used, for ffmpeg
    int type;
    void *ad_priv;
    struct ad_wrapper *next;
    void *parent;
} ad_wrapper_t;

typedef struct dtaudio_decoder {
    dtaudio_para_t para;
    ad_wrapper_t *wrapper;
    pthread_t audio_decoder_pid;
    adec_status_t status;
    int decode_err_cnt;
    int decode_offset;

    int64_t pts_current;
    int64_t pts_first;
    int first_frame_decoded;
    int64_t pts_last_valid;
    int pts_buffer_size;
    int pts_cache_size;

    adec_ctrl_t info;
    dt_buffer_t *buf_out;
    void *parent;
    void *decoder_priv;         //point to avcodeccontext
} dtaudio_decoder_t;

#endif

#ifndef DT_STATE_T
#define DT_STATE_T

#include "stdint.h"

typedef struct {
    int size;
    int data_len;
    int free_len;
    int total_len;
} buf_state_t;

typedef struct dec_state {
    int adec_channels;
    int adec_sample_rate;
    int adec_bps;
    int adec_error_count;
    int adec_status;

    int vdec_width;
    int vdec_height;
    int vdec_fps;
    int vdec_error_count;
    int vdec_status;

    int sdec_width;
    int sdec_height;
    int sdec_error_count;
    int sdec_status;
} dec_state_t;

#if 0
typedef struct {
    int abuf_level;
    int vbuf_level;

    int adec_err_cnt;
    int vdec_err_cnt;

    int64_t cur_apts;
    int64_t cur_vpts;
    int64_t cur_systime;

} host_state_t;
#endif
#endif

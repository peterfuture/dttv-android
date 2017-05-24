#ifndef DTPLAYER_STATE_H
#define DTPLAYER_STATE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "dtp_av.h"

#define MAX_VIDEO_STREAM_NUM 5
#define MAX_AUDIO_STREAM_NUM 20
#define MAX_SUBTITLE_STREAM_NUM 20
#define LANGUAGE_MAX_SIZE 1024

typedef enum {
    DT_SYNC_AUDIO_MASTER,
    DT_SYNC_VIDEO_MASTER,
    DT_SYNC_EXTERNAL_CLOCK,
}
dtp_sync_mode_t;

typedef enum {
    DTP_CMD_SET_AODEVICE = 0x100,
    DTP_CMD_SET_VODEVICE,
    DTP_CMD_SET_SODEVICE,
} dtp_cmd_t;

typedef struct {
    int num;
    int den;
} dtp_ratio_t;

typedef struct {
    int index;
    int id;
    int bit_rate;
    int width;
    int height;
    int pix_fmt;
    int64_t duration;
    dtp_ratio_t sample_aspect_ratio; //witdh:height
    dtp_ratio_t frame_rate_ratio;
    dtp_ratio_t time_base;
    int extradata_size;
    uint8_t extradata[VIDEO_EXTRADATA_SIZE];
    char language[LANGUAGE_MAX_SIZE];
    dtvideo_format_t format;
    void *codec_priv;
} vstream_info_t;

typedef struct {
    char title[512];
    char author[512];
    char album[512];
    char comment[512];
    char year[4];
    int track;
    char genre[32];
    char copyright[512];
    int cover_type; // 0-none 1-jpg 2-png
} album_info_t;

typedef struct {
    int index;
    int id;
    int bit_rate;
    int sample_rate;
    int channels;
    int bps;
    int64_t duration;
    dtp_ratio_t time_base;
    int extradata_size;
    uint8_t *extradata;
    char language[LANGUAGE_MAX_SIZE];
    dtaudio_format_t format;
    album_info_t album_info;
    void *codec_priv;
} astream_info_t;

typedef struct {
    int index;
    int id;
    int bit_rate;
    int width;
    int height;
    int extradata_size;
    uint8_t *extradata;
    char language[LANGUAGE_MAX_SIZE];
    dtsub_format_t format;
    void *codec_priv;
} sstream_info_t;

typedef struct {
    int vst_num;
    int ast_num;
    int sst_num;
    vstream_info_t *vstreams[MAX_VIDEO_STREAM_NUM];
    astream_info_t *astreams[MAX_AUDIO_STREAM_NUM];
    sstream_info_t *sstreams[MAX_SUBTITLE_STREAM_NUM];
} track_info_t;

typedef struct {
    char *file;
    int64_t start_time;
    int64_t duration;
    int64_t file_size;
    int bit_rate;
    dtp_media_format_t format;
    int livemode;

    int nb_stream;
    int has_video;
    int cur_vst_index;
    int disable_video;
    int has_audio;
    int cur_ast_index;
    int disable_audio;
    int has_sub;
    int cur_sst_index;
    int disable_sub;

    track_info_t tracks;
} dtp_media_info_t;

typedef enum {
    PLAYER_STATUS_INVALID = -1,
    PLAYER_STATUS_IDLE,

    PLAYER_STATUS_INIT_ENTER,
    PLAYER_STATUS_INIT_EXIT,

    PLAYER_STATUS_PREPARE_START,
    PLAYER_STATUS_PREPARED,

    PLAYER_STATUS_START,
    PLAYER_STATUS_RUNNING,

    PLAYER_STATUS_PAUSED,
    PLAYER_STATUS_RESUME,
    PLAYER_STATUS_SEEK_ENTER,
    PLAYER_STATUS_SEEK_EXIT,

    PLAYER_STATUS_ERROR,
    PLAYER_STATUS_STOP,
    PLAYER_STATUS_PLAYEND,
    PLAYER_STATUS_EXIT,
} player_status_t;

typedef struct {
    int size;
    int data_len;
    int free_len;
    int total_len;
} buf_state_t;

typedef struct {
    int adec_channels;
    int adec_sample_rate;
    int adec_bps;
    int adec_error_count;
    int adec_status;
    int64_t adec_last_ms;

    int vdec_width;
    int vdec_height;
    int vdec_fps;
    int vdec_error_count;
    int vdec_status;
    int64_t vdec_last_ms;
    int vdec_type;

    int sdec_width;
    int sdec_height;
    int sdec_error_count;
    int sdec_status;
} dec_state_t;

typedef struct {
    /* player state */
    player_status_t cur_status;
    player_status_t last_status;

    int64_t cur_time_ms;
    int64_t cur_time;
    int64_t full_time;
    int64_t start_time;
    int64_t discontinue_point_ms;

    int vdec_type;
} dtp_state_t;

#ifdef  __cplusplus
}
#endif

#endif

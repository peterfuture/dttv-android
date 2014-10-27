#ifndef DTPLAYER_PARA_H
#define DTPLAYER_PARA_H

#include "dt_media_info.h"

typedef enum
{
    PLAYER_STATUS_INVALID = -1,
    PLAYER_STATUS_IDLE,

    PLAYER_STATUS_INIT_ENTER,
    PLAYER_STATUS_INIT_EXIT,

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

typedef struct
{
    /* player state */
    player_status_t cur_status;
    player_status_t last_status;

    int64_t cur_time_ms;
    int64_t cur_time;
    int64_t full_time;

    int64_t start_time;
} player_state_t;

typedef struct dtplayer_para
{
    char *file_name;
    int video_index;
    int audio_index;
    int sub_index;

    int loop_mode;
    int disable_audio;
    int disable_video;
    int disable_sub;
    int disable_avsync;
    int disable_hw_acodec;
    int disable_hw_vcodec;
    int disable_hw_scodec;

    int width;
    int height;
    
    void *cookie;
    int (*update_cb) (void *cookie, player_state_t * sta);
} dtplayer_para_t;

#endif

#ifndef DTPLAYER_API_H
#define DTPLAYER_API_H

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
    int no_audio;
    int no_video;
    int no_sub;
    int sync_enable;

    int width;
    int height;

    int (*update_cb) (player_state_t * sta);
} dtplayer_para_t;

/* 
 * DTPLAYER API DEFINITION
 *
 * */

/* 
 * do global initialization of dtplayer.
 * finish format detect, stream-demuxer init, etc
 *
 * @param para - parameter setup by caller
 * @return handle - pointer to dtplayer handle which used for do more contrl works
 *
 * */
void *dtplayer_init (dtplayer_para_t * para);

/*
 * mediainfo:
 * get all media information using this api
 * need to be called after dtplayer_init
 *
 * @param handle - dtplayer handle
 * @param info - mediainfo structure, store
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_get_mediainfo (void *handle, dt_media_info_t *info);

/*
 * start playing:
 * data fill loop 
 * decode-render process
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_start (void *handle);

/*
 * pause player:
 * pause render thread, reset part will block
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_pause (void *handle);

/*
 * resume player:
 * resume render thread
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_resume (void *handle);

/*
 * stop player:
 * quit stream-demuxer-decoder-render module
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_stop (void *handle);

/*
 * seek:
 * seek to (cur_time + step), not recommended, will remove later
 *
 * @param handle - dtplayer handle
 * @param step - step
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_seek (void *handle, int step);

/*
 * seekto:
 * seek to dest_pos, recommended API
 *
 * @param handle - dtplayer handle
 * @param dest_pos - dest position time (s)
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_seekto (void *handle, int dest_pos);

/*
 * dtplayer states update:
 * called by user, get dtplayer states, for more details refer to player_state_t definition
 *
 * @param handle - dtplayer handle
 * @param state 
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_get_states (void *handle, player_state_t * state);

#endif

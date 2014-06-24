#ifndef DTPLAYER_H
#define DTPLAYER_H

#include "dtplayer_api.h"
#include "dt_media_info.h"
#include "dthost_api.h"
#include "dt_av.h"
#include "dt_event.h"

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
    int status;                 // 0 start 1 pause 2 quit
    int flag;
    pthread_t tid;
} io_loop_t;

typedef struct
{
    /*stream info */
    int64_t start_time;
    int64_t first_time;

    int cur_ast_index;
    int cur_vst_index;
    int cur_sst_index;

    /*dest width height */
    int width;
    int height;
    /*ctrl flag */
    int eof_flag;
    int sync_enable;
    int has_audio;
    int has_video;
    int has_sub;

} player_ctrl_t;

typedef struct dtplayer_context
{
    char *file_name;
    dtplayer_para_t player_para;

    void *demuxer_priv;
    dt_media_info_t *media_info;

    player_ctrl_t ctrl_info;
    dthost_para_t host_para;
    void *host_priv;

    player_state_t state;
    int (*update_cb) (player_state_t * state); // update player info to uplevel

    io_loop_t io_loop;
    pthread_t event_loop_id;

    void *player_server;
} dtplayer_context_t;

void player_register_all();
int player_init (dtplayer_context_t * dtp_ctx);
int player_start (dtplayer_context_t * dtp_ctx);
int player_pause (dtplayer_context_t * dtp_ctx);
int player_resume (dtplayer_context_t * dtp_ctx);
int player_seekto (dtplayer_context_t * dtp_ctx, int seek_time); //s
int player_get_mediainfo (dtplayer_context_t * dtp_ctx, dt_media_info_t *info); //s
int player_stop (dtplayer_context_t * dtp_ctx);

#endif

#ifndef DT_SETTING_H
#define DT_SETTING_H

typedef struct dt_setting {
    // LOG
    int log_level;
    char log_filter[1024];
    // STREAM
    int stream_cache;
    int stream_cache_size;
    // DEMUXER
    int demuxer_probe;
    int demuxer_probe_size;
    int demuxer_seek_keyframe;
    // AUDIO
    int audio_downmix;
    int audio_index;
    //VIDEO
    int video_pts_mode;
    int video_pixel_format;
    int video_index;
    int video_render_mode;
    int video_render_duration;
    // SUB
    int sub_index;
    // PLAYER
    int player_noaudio;
    int player_novideo;
    int player_nosub;
    int player_sync_enable;
    int player_dump_mode;
    int player_seekmode;

    // HOST
    int host_drop;
    int host_drop_thres;
    int host_sync_thres;
} dt_setting_t;

extern dt_setting_t dtp_setting;

int dt_update_setting();

#endif

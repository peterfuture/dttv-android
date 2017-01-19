#ifndef DTP_H
#define DTP_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "dtp_av.h"
#include "dtp_state.h"
#include "dtp_plugin.h"

typedef int (*dtp_update_cb)(void *cookie, dtp_state_t * sta);

typedef struct dtplayer_para {
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
    int video_pixel_format;

    int width;
    int height;

    dtp_update_cb update_cb;
    void *cookie;
} dtplayer_para_t;

/*
 * DTP - API
 *
 * */

/*
 * register external plugin to dtplayer
 *
 * */
void dtplayer_register_plugin(dtp_plugin_type_t type, void *plugin);

/*
 * do global initialization of dtplayer.
 * finish format detect, stream-demuxer init, etc
 *
 * @param para - parameter setup by caller
 * @return handle - pointer to dtplayer handle which used for do more contrl works
 *
 * */
void *dtplayer_init(dtplayer_para_t * para);

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
int dtplayer_get_mediainfo(void *handle, dtp_media_info_t *info);

/*
 * query dtplayer information:
 *
 * @param handle - dtplayer handle
 * @param cmd - query info type
 * @param reply - reply from dtp
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_get_parameter(void *handle, int cmd, unsigned long reply);

/*
 * affect dtplayer with request:
 *
 * @param handle - dtplayer handle
 * @param cmd - query info type
 * @param request - parameter from user
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_set_parameter(void *handle, int cmd, unsigned long request);

/*
 * start playing:
 * data fill loop
 * decode-render process
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_start(void *handle);

/*
 * pause player:
 * pause render thread, reset part will block
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_pause(void *handle);

/*
 * resume player:
 * resume render thread
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_resume(void *handle);

/*
 * stop player:
 * quit stream-demuxer-decoder-render module
 *
 * @param handle - dtplayer handle
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_stop(void *handle);

/*
 * seekto:
 * seek to dest_pos, recommended API
 *
 * @param handle - dtplayer handle
 * @param dest_pos - dest position time (s)
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_seekto(void *handle, int dest_pos);

/*
 * dtplayer states update:
 * called by user, get dtplayer states, for more details refer to dtp_state_t definition
 *
 * @param handle - dtplayer handle
 * @param state
 * @return ret - 0 success , negtive failed
 *
 * */
int dtplayer_get_states(void *handle, dtp_state_t * state);

#ifdef  __cplusplus
}
#endif

#endif

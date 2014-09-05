#ifndef DTPLAYER_API_H
#define DTPLAYER_API_H

#include "dtplayer_para.h"
#include "stream_wrapper.h"
#include "demuxer_wrapper.h"
#include "ad_wrapper.h"
#include "ao_wrapper.h"
#include "vd_wrapper.h"
#include "vo_wrapper.h"
#include "vf_wrapper.h"
/* 
 * DTPLAYER API DEFINITION
 *
 * */

/* 
 * register external module
 * including ao vo ad vd stream demuxer
 * supporting third-party develop
 *
 * */

void dtplayer_register_ext_stream(stream_wrapper_t *wrapper);
void dtplayer_register_ext_demuxer(demuxer_wrapper_t *wrapper);
void dtplayer_register_ext_ao(ao_wrapper_t *wrapper);
void dtplayer_register_ext_ad(ad_wrapper_t *wrapper);
void dtplayer_register_ext_vo(vo_wrapper_t *wrapper);
void dtplayer_register_ext_vd(vd_wrapper_t *wrapper);
void dtplayer_register_ext_vf(vf_wrapper_t *wrapper);

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
 * set Video Size: 
 *
 * @param handle - dtplayer handle
 * @param width  - dst width
 * @param height - dst height
 * @return ret - 0 success , negtive failed
 * 
 * */
int dtplayer_set_video_size (void *handle, int w, int h);

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

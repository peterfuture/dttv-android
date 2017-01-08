#ifndef DTVIDEO_FILT_H
#define DTVIDEO_FILT_H

#include "vf_wrapper.h"

void vf_register_all();
void vf_remove_all();
void vf_register_ext(vf_wrapper_t *vf);

int video_filter_init(dtvideo_filter_t * filter);
int video_filter_update(dtvideo_filter_t * filter);
int video_filter_process(dtvideo_filter_t * filter, dt_av_frame_t *pic);
int video_filter_stop(dtvideo_filter_t * filter);

#endif

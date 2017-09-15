//
// Created by dttv on 16-5-3.
//

#ifndef GL_RENDER_YUV_H
#define GL_RENDER_YUV_H

#include "dtp_av.h"

void yuv_dttv_init();

int yuv_update_frame(dt_av_frame_t *frame);

bool yuv_setupGraphics(int w, int h);

void yuv_renderFrame();

#endif

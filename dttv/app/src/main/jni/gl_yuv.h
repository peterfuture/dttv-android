//
// Created by dttv on 16-5-3.
//

#ifndef GLES2JNI_GL_YUV_H
#define GLES2JNI_GL_YUV_H

#include "../../../../3rd/libdtp/include/dtp_av.h"

void yuv_dttv_init();

void yuv_reg_player(void *mp);

int yuv_update_frame(dt_av_frame_t *frame);

bool yuv_setupGraphics(int w, int h);

void yuv_renderFrame();

#endif //GLES2JNI_GL_YUV_H

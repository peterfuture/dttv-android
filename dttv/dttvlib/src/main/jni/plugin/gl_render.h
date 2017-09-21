#ifndef GL_RENDER_H
#define GL_RENDER_H

#include <dtp.h>
#include <dttv_jni_cmd.h>

int gl_create(void *h);
int gl_setup(int w, int h);
int gl_get_parameter(int cmd, unsigned long arg);
int gl_set_parameter(int cmd, unsigned long arg1, unsigned long arg2);
int gl_notify();
int gl_update_frame(dt_av_frame_t *frame);
int gl_render();

#endif
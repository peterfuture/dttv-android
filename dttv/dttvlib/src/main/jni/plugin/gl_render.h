#ifndef GL_RENDER_H
#define GL_RENDER_H

#include <dtp.h>

int gl_create(void *h, int pixfmt);
int gl_setup(int w, int h);
int gl_notify();
int gl_update_frame(dt_av_frame_t *frame);
int gl_render();

#endif
//
// Created by dttv on 16-5-3.
//

#ifndef GL_RENDER_RGB_H
#define GL_RENDER_RGB_H

void rgb_init();
int rgb_setup(int w, int h);
int rgb_update_frame(dt_av_frame_t *frame);
void rgb_render();

#endif

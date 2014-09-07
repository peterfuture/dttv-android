#ifndef DTVIDEO_PIC_H
#define DTVIDEO_PIC_H

#include "dt_av.h"

dt_av_pic_t *dtav_new_pic();
int dtav_unref_pic(dt_av_pic_t *pic);
int dtav_free_pic(dt_av_pic_t *pic);

#endif

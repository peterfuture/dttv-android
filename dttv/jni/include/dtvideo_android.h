//for video
#ifndef DTVIDEO_ANDROID_H
#define DTVIDEO_ANDROID_H

#include <stdint.h>
#include "vo_wrapper.h"
#if 0
/* AVPicture_t is wrapper of AVPicture */
typedef struct _AVPicture_t_
{
    /* The two fields is same as AVPicture */
    uint8_t *data[8];
    int linesize[8];
    /* New fields for sync AV */
    int64_t pts;
    int64_t dts;
    int duration;
} AVPicture_t;

typedef struct vo_wrapper
{
    int id;
    char *name;

    int (*vo_init) ();
    int (*vo_stop) ();
    int (*vo_render) (AVPicture_t * pic);
    void *handle;
    struct vo_wrapper *next;
    void *vo_priv;
} vo_wrapper_t;
#endif
#endif

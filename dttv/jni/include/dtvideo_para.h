#ifndef DTVIDEO_PARA_H
#define DTVIDEO_PARA_H

#include <stdint.h>
#define VIDEO_EXTRADATA_SIZE 4096
typedef struct
{
    int vfmt;
    int d_width;                //dest w
    int d_height;               //dest h
    int s_width;                //src w
    int s_height;               //src h
    int d_pixfmt;               //dest pixel format
    int s_pixfmt;               //src pixel format
    int rate;
    int ratio;
    double fps;
    int num, den;               //for pts calc
    int extradata_size;
    unsigned char extradata[VIDEO_EXTRADATA_SIZE];
    int flag;                   // setting from player
    int video_filter;
    int video_output;
    void *avctx_priv;
} dtvideo_para_t;

#endif

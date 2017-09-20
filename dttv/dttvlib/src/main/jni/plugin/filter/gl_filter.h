#ifndef GL_FILTER_H
#define GL_FILTER_H

#include <gl_filter_yuv.h>
#include <gl_filter_rgb.h>

typedef enum {
    GL_FILTER_TYPE_YUV = 0,
    GL_FILTER_TYPE_RGB,
    GL_FILTER_TYPE_SATURATION,
} gl_filter_type;

#endif
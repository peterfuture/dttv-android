#ifndef DTTV_JNI_CMD_H
#define DTTV_JNI_CMD_H

typedef enum {
    GL_FILTER_TYPE_YUV = 0,
    GL_FILTER_TYPE_RGB,
    GL_FILTER_TYPE_SATURATION,
} gl_filter_type;

// Need to match with Java cmd definitions
typedef enum {
    KEY_PARAMETER_USEHWCODEC = 0x0,
    KEY_PARAMETER_SET_GLFILTER = 0x1,
} dttv_jni_cmd_t;

#endif

#ifndef STREAM_WRAPPER_H
#define STREAM_WRAPPER_H

#include "dt_av.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    STREAM_INVALID = -1,
    STREAM_FILE,
    STREAM_CURL,
    STREAM_FFMPEG,
    STREAM_CACHE, // online video only
    STREAM_UNSUPPORT
} stream_format_t;

typedef struct {
    int is_stream;
    int seek_support;
    int64_t cur_pos;
    int64_t stream_size;
    int eof_flag;
} stream_ctrl_t;

typedef struct stream_wrapper {
    char *name;
    int id;
    int (*open)(struct stream_wrapper * wrapper, char *stream_name);
    int64_t (*tell)(struct stream_wrapper * wrapper);
    int (*read)(struct stream_wrapper * wrapper, uint8_t *buf, int len);
    int (*seek)(struct stream_wrapper * wrapper, int64_t pos, int whence);
    int (*close)(struct stream_wrapper * wrapper);
    void *stream_priv;          // point to priv context
    void *parent;               // point to parent, dtstream_context_t
    stream_ctrl_t info;
    struct stream_wrapper *next;
} stream_wrapper_t;

typedef struct {
    char *stream_name;
    stream_wrapper_t *stream;
    void *parent;
} dtstream_context_t;

#endif

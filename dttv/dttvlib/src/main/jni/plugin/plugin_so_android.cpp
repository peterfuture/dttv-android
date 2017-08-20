//
// Created by peterfuture on 2017/8/19.
//

#include <dtp_sub_plugin.h>
#include "dttv_jni_log.h"

#define TAG "SO-ANDROID"

typedef struct so_android_ctx {
    int id;
} so_android_ctx;

static int so_android_init(so_context_t *soc) {
    LOGV("init so android OK\n");
    return 0;
}

static int so_android_stop(so_context_t *soc) {
    LOGV("stop so android\n");
    return 0;
}

static int so_android_render(so_context_t *soc, dtav_sub_frame_t *frame) {
    LOGV("sub render ok\n");
    return 0;
}

so_wrapper_t so_android_ops = {
        .id = SO_ID_ANDROID,
        .name = "android so",
        .so_init = so_android_init,
        .so_stop = so_android_stop,
        .so_render = so_android_render,
        .private_data_size = sizeof(so_android_ctx),
};
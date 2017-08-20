/*
 * =====================================================================================
 *
 *    Filename   :  dtp_sub_plugin.h
 *    Description:
 *    Version    :  1.0
 *    Created    :  2017年08月16日 20时42分19秒
 *    Revision   :  none
 *    Compiler   :  gcc
 *    Author     :  peter-s (), peter_future@outlook.com
 *    Company    :  dt
 *
 * =====================================================================================
 */

#ifndef DTP_SUB_PLUGIN_H
#define DTP_SUB_PLUGIN_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "dtp_av.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

struct so_wrapper;
struct so_context;

#define SUB_EXTRADATA_SIZE 4096

typedef struct {
    int sfmt;
    int width;
    int height;
    int sub_output;
    void *device;         // sub render device
    void *avctx_priv;
} dtsub_para_t;

typedef struct so_wrapper {
    int id;
    const char *name;

    int (*so_init)(struct so_context *soc);
    int (*so_stop)(struct so_context *soc);
    int (*so_render)(struct so_context *soc, dtav_sub_frame_t * frame);
    void *handle;
    struct so_wrapper *next;
    int private_data_size;
} so_wrapper_t;

typedef struct so_context {
    dtsub_para_t para;
    so_wrapper_t *wrapper;
    void *private_data;
} so_context_t;

#ifdef  __cplusplus
}
#endif

#endif

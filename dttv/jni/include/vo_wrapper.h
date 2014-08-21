#ifndef VO_WRAPPER_H
#define VO_WRAPPER_H

#include "dtvideo_para.h"
#include "dt_av.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

//declare
struct dtvideo_output;

typedef struct vo_wrapper
{
    int id;
    char *name;

    int (*vo_init) (struct dtvideo_output *vout);
    int (*vo_stop) (struct dtvideo_output *vout);
    int (*vo_render) (struct dtvideo_output *vout,AVPicture_t * pic);
    void *handle;
    struct vo_wrapper *next;
    void *vo_priv;
} vo_wrapper_t;

typedef enum
{
    VO_STATUS_IDLE,
    VO_STATUS_PAUSE,
    VO_STATUS_RUNNING,
    VO_STATUS_EXIT,
} vo_status_t;

typedef enum _VO_ID_
{
    VO_ID_EXAMPLE = -1,
    VO_ID_EX,            // ex vo need set to 0 default
    VO_ID_SDL,
    VO_ID_X11,
    VO_ID_FB,
    VO_ID_GL,
    VO_ID_DIRECTX,
    VO_ID_SDL2,
    VO_ID_ANDROID = 0x100,
    VO_ID_IOS = 0x200,
} dt_vo_t;

typedef struct
{
    int vout_buf_size;
    int vout_buf_level;
} vo_state_t;

typedef struct dtvideo_output
{
    /*param */
    dtvideo_para_t para;
    vo_wrapper_t *wrapper;
    vo_status_t status;
    pthread_t output_thread_pid;
    vo_state_t state;

    uint64_t last_valid_latency;
    void *parent;               //point to dtvideo_output_t
}dtvideo_output_t;

#endif

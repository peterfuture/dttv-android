#ifndef SO_WRAPPER_H
#define SO_WRAPPER_H

#include "dtsub_para.h"
#include "dt_av.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

//declare
struct dtsub_output;

typedef struct so_wrapper
{
    int id;
    const char *name;

    int (*so_init) (struct dtsub_output *sout);
    int (*so_stop) (struct dtsub_output *sout);
    int (*so_render) (struct dtsub_output *sout,dtav_sub_frame_t * frame);
    void *handle;
    struct so_wrapper *next;
    void *so_priv;
}so_wrapper_t;

typedef enum
{
    SO_STATUS_IDLE,
    SO_STATUS_PAUSE,
    SO_STATUS_RUNNING,
    SO_STATUS_EXIT,
}so_status_t;

typedef enum _so_ID_
{
    SO_ID_INVALID = -1,
    SO_ID_NULL    = 0,
}dt_so_t;

typedef struct
{
    int sout_buf_size;
    int sout_buf_level;
}so_state_t;

typedef struct dtsub_output
{
    dtsub_para_t *para;
    so_wrapper_t *wrapper;
    so_status_t status;
    pthread_t output_thread_pid;
    so_state_t state;

    void *parent;               //point to dtsub_output_t
}dtsub_output_t;

#endif

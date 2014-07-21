#ifndef AO_WRAPPER_H
#define AO_WRAPPER_H

#include "dt_av.h"
#include "dtaudio_para.h"

#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

typedef struct ao_wrapper
{
    int id;
    char *name;
    dtaudio_para_t para;
    
    int (*ao_init) (dtaudio_para_t *para);
    int (*ao_start) ();
    int (*ao_pause) ();
    int (*ao_resume) ();
    int (*ao_stop) ();
    int64_t (*ao_latency) ();
    int (*ao_level) ();
    int (*ao_write) (uint8_t * buf, int size);
    struct ao_wrapper *next;
    void *ao_priv;
} ao_wrapper_t;

#define dtao_format_t ao_id_t

typedef enum
{
    AO_STATUS_IDLE,
    AO_STATUS_PAUSE,
    AO_STATUS_RUNNING,
    AO_STATUS_EXIT,
} ao_status_t;

typedef enum _AO_CTL_ID_
{
    AO_GET_VOLUME,
    AO_ADD_VOLUME,
    AO_SUB_VOLUME,
    AO_CMD_PAUSE,
    AO_CMD_RESUME,
} ao_cmd_t;

typedef struct
{
    int aout_buf_size;
    int aout_buf_level;
} ao_state_t;

typedef struct dtaudio_output
{
    /*para */
    dtaudio_para_t para;
    ao_wrapper_t *aout_ops;
    ao_status_t status;
    pthread_t output_thread_pid;
    ao_state_t state;

    uint64_t last_valid_latency;
    void *parent;               //point to dtaudio_t, can used for param of pcm get interface
}dtaudio_output_t;


#endif

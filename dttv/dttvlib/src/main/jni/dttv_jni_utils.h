#ifndef DTTV_JNI_UTILS_H
#define DTTV_JNI_UTILS_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define lock_t         pthread_mutex_t
#define lock_init(x,v) pthread_mutex_init(x,v)
#define lock(x)        pthread_mutex_lock(x)
#define unlock(x)      pthread_mutex_unlock(x)

typedef struct {
    uint8_t *data;
    int size;
    int level;
    uint8_t *rd_ptr;
    uint8_t *wr_ptr;
    lock_t mutex;
} dt_buffer_t;

int buf_init(dt_buffer_t * dbt, int size);
int buf_reinit(dt_buffer_t * dbt);
int buf_release(dt_buffer_t * dbt);
int buf_space(dt_buffer_t * dbt);
int buf_level(dt_buffer_t * dbt);
int buf_get(dt_buffer_t * dbt, uint8_t * out, int size);
int buf_put(dt_buffer_t * dbt, uint8_t * in, int size);

#ifdef  __cplusplus
}
#endif

#endif

#ifndef DT_BUFFER_T
#define DT_BUFFER_T

#include "dt_lock.h"
#include "dt_macro.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t *data;
    int size;
    int level;
    uint8_t *rd_ptr;
    uint8_t *wr_ptr;
    dt_lock_t mutex;
} dt_buffer_t;

int buf_init(dt_buffer_t * dbt, int size);
int buf_reinit(dt_buffer_t * dbt);
int buf_release(dt_buffer_t * dbt);
int buf_space(dt_buffer_t * dbt);
int buf_level(dt_buffer_t * dbt);
int buf_get(dt_buffer_t * dbt, uint8_t * out, int size);
int buf_put(dt_buffer_t * dbt, uint8_t * in, int size);
#endif

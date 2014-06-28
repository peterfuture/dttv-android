#ifndef DT_LOCK_H
#define DT_LOCK_H

#include "pthread.h"

#define dt_lock_t         pthread_mutex_t
#define dt_lock_init(x,v) pthread_mutex_init(x,v)
#define dt_lock(x)        pthread_mutex_lock(x)
#define dt_unlock(x)      pthread_mutex_unlock(x)

#endif

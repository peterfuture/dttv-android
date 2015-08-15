#ifndef DT_EVENT_H
#define DT_EVENT_H

#include "dt_lock.h"

typedef struct event {
    int type;
    union {
        int np;
    } para;
    int server_id;
    struct event *next;
} event_t;

typedef struct event_server {
    char name[1024];
    int id;
    event_t *event;
    dt_lock_t event_lock;
    int event_count;
    struct event_server *next;
} event_server_t;

typedef struct event_server_mgt {
    event_server_t *server;
    dt_lock_t server_lock;
    int server_count;
    int exit_flag;
    pthread_t transport_loop_id;
} dt_server_mgt_t;

int dt_event_server_init();
int dt_event_server_release();
event_server_t *dt_alloc_server();
int dt_register_server(event_server_t * server);
int dt_remove_server(event_server_t * server);
event_t *dt_alloc_event();
int dt_send_event_sync(event_t * event);
int dt_send_event(event_t * event);
int dt_add_event(event_t * event, event_server_t * server);
event_t *dt_get_event(event_server_t * server);
event_t *dt_peek_event(event_server_t * server);

#endif

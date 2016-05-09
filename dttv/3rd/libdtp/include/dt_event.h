#ifndef DT_EVENT_H
#define DT_EVENT_H

#include "dt_lock.h"

typedef struct event {
    int type;
    union {
        int np;
    } para;
    int service_id;
    struct event *next;
} event_t;

#define MAX_EVENT_SERVER_NAME_LEN 1024

typedef struct service {
    char name[MAX_EVENT_SERVER_NAME_LEN];
    int id;
    event_t *event;
    dt_lock_t event_lock;
    int event_count;
    struct service *next;
} service_t;

#define EVENT_SERVER_ID_MAIN   0
#define EVENT_SERVER_NAME_MAIN "SERVER-MAIN"

typedef struct service_mgt {
    service_t *service;
    dt_lock_t service_lock;
    int service_count;
    int exit_flag;
    pthread_t transport_loop_id;
} dt_service_mgt_t;


/* *
 * Create service manager context
 *
 * @param cache cache context
 * @param size FIFO Size
 * @param mode cache mode
 *
 * @return dt_service_mgt_t pointer for success, NULL otherwise
 *
 */
dt_service_mgt_t *dt_service_create();

/* *
 * Release service manager context
 * Just release service manager and main service, guarantee
 * have removed all other service but service-main
 *
 * @param mgt service manager context
 *
 * @return 0 for success, negative errorcode otherwise
 *
 */
int dt_service_release(dt_service_mgt_t *mgt);

/* *
 * Alloc service context
 *
 * @param id   User defined service id
 * @param name User defined service name
 *
 * @return service_t pointer for success, NULL otherwise
 *
 */
service_t *dt_alloc_service(int id, char *name);

/* *
 * Register service to service manager
 *
 * @param mgt    service manager context
 * @param service service to be register
 *
 * @return 0 for success, negative errorcode otherwise
 *
 */
int dt_register_service(dt_service_mgt_t *mgt, service_t * service);

/* *
 * Remove service from service manager
 * remove event in service at the same time
 *
 * @param mgt    service manager context
 * @param service service to be removed
 *
 * @return 0 for success, negative errorcode otherwise
 *
 */
int dt_remove_service(dt_service_mgt_t *mgt, service_t * service);

/* *
 * Alloc an event_t
 *
 * @param service event service to be sent
 * @param type   event type
 *
 * @return event_t pointer for success, NULL otherwise
 *
 */
event_t *dt_alloc_event(int service, int type);

/* *
 * Send event to service manger, and make sure event
 * transport dest service before return
 *
 * @param mgt   service manager context
 * @param event event to be sent
 *
 * @return 0 for success, negative errorcode otherwise
 *
 */
int dt_send_event_sync(dt_service_mgt_t *mgt, event_t * event);

/* *
 * Send event to service manger - non block
 *
 * @param mgt   service manager context
 * @param event event to be sent
 *
 * @return 0 for success, negative errorcode otherwise
 *
 */
int dt_send_event(dt_service_mgt_t *mgt, event_t * event);

/* *
 * Get event from service
 *
 * @param service service context
 *
 * @return first event pointer for success, NULL if no event in service
 * remove event from service
 *
 */
event_t *dt_get_event(service_t * service);

/* *
 * Query event from service
 *
 * @param service service context
 *
 * @return first event pointer for success, NULL if no event in service
 * not remove event from service
 *
 */
event_t *dt_peek_event(service_t * service);

#endif

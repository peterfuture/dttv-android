#ifndef DTUTIL_TIME_H
#define DTUTIL_TIME_H

#include <stdint.h>

/**
 * Get the current time in microseconds.
 */
int64_t dt_gettime(void);

/**
 * Sleep for a period of time.  Although the duration is expressed in
 * microseconds, the actual delay may be rounded to the precision of the
 * system timer.
 *
 * @param  usec Number of microseconds to sleep.
 * @return zero on success or (negative) error code.
 */
int dt_usleep(unsigned usec);

#endif /* DTUTIL_TIME_H */

/*
 * timer.c — clock_gettime(CLOCK_MONOTONIC) wrapper
 */

#include "timer.h"

#include <time.h>

uint64_t timer_now_ns(void) {
    struct timespec ts;
    /* CLOCK_MONOTONIC is the right choice for interval timing.
     * It is not affected by wall-clock adjustments. */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}
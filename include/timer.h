/*
 * timer.h — high-resolution monotonic timestamps
 */
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*
 * Returns the current value of CLOCK_MONOTONIC in nanoseconds.
 * Monotonic means: never goes backwards, unaffected by NTP/clock
 * adjustments. Suitable for measuring intervals.
 */
uint64_t timer_now_ns(void);

/* Convert nanoseconds to a printable microsecond float. */
static inline double ns_to_us(uint64_t ns) {
    return (double)ns / 1000.0;
}

#endif /* TIMER_H */
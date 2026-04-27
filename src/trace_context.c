/*
 * trace_context.c — accumulate syscall statistics
 */

#include "trace_context.h"
#include <string.h>

void tc_init(trace_context_t *tc) {
    memset(tc, 0, sizeof(*tc));
    /* Initialize min_ns to UINT64_MAX so the first observation
     * always wins the comparison. */
    for (int i = 0; i < TC_MAX_SYSCALL; i++) {
        tc->per_syscall[i].min_ns = UINT64_MAX;
    }
}

void tc_record(trace_context_t *tc, long syscall_nr, uint64_t latency_ns) {
    if (syscall_nr < 0 || syscall_nr >= TC_MAX_SYSCALL) {
        /* Unknown number — still count it in totals and "Other". */
        tc->total_count++;
        tc->total_ns += latency_ns;
        tc->per_category[CAT_OTHER].count++;
        tc->per_category[CAT_OTHER].total_ns += latency_ns;
        return;
    }

    per_syscall_stats_t *s = &tc->per_syscall[syscall_nr];
    s->count++;
    s->total_ns += latency_ns;
    if (latency_ns < s->min_ns) s->min_ns = latency_ns;
    if (latency_ns > s->max_ns) s->max_ns = latency_ns;

    category_t c = classify(syscall_nr);
    tc->per_category[c].count++;
    tc->per_category[c].total_ns += latency_ns;

    tc->total_count++;
    tc->total_ns += latency_ns;
}
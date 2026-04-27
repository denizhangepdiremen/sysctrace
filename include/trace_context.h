/*
 * trace_context.h — per-syscall and per-category statistics
 *
 * For each of the 335 syscall numbers we track count, total time,
 * min, and max latency. We also keep per-category aggregates so the
 * reporter can render the summary table directly.
 */
#ifndef TRACE_CONTEXT_H
#define TRACE_CONTEXT_H

#include "classifier.h"
#include <stdint.h>

#define TC_MAX_SYSCALL 335

typedef struct {
    uint64_t count;
    uint64_t total_ns;
    uint64_t min_ns;
    uint64_t max_ns;
} per_syscall_stats_t;

typedef struct {
    uint64_t count;
    uint64_t total_ns;
} per_category_stats_t;

typedef struct trace_context {
    per_syscall_stats_t  per_syscall[TC_MAX_SYSCALL];
    per_category_stats_t per_category[CAT_COUNT];
    uint64_t             total_count;
    uint64_t             total_ns;       /* sum of all syscall latencies */
    uint64_t             wall_start_ns;  /* tracee start timestamp */
    uint64_t             wall_end_ns;    /* tracee end timestamp   */
} trace_context_t;

/* Zero-initialize the context. */
void tc_init(trace_context_t *tc);

/* Record one finished syscall (entry+exit observed). */
void tc_record(trace_context_t *tc, long syscall_nr, uint64_t latency_ns);

#endif /* TRACE_CONTEXT_H */
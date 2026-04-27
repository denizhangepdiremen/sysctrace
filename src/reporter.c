/*
 * reporter.c — terminal report rendering
 *
 * Two sections:
 *   1) Per-category summary (5 rows, one per category)
 *   2) Top-N most-frequent syscalls with count, total, avg, min, max
 */

#include "reporter.h"
#include "syscall_table.h"
#include "timer.h"
#include "classifier.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long                nr;
    per_syscall_stats_t s;
} row_t;

/* qsort comparator: descending by count. */
static int cmp_by_count(const void *a, const void *b) {
    const row_t *ra = (const row_t *)a;
    const row_t *rb = (const row_t *)b;
    if (rb->s.count > ra->s.count) return 1;
    if (rb->s.count < ra->s.count) return -1;
    return 0;
}

static void render_header(FILE *out) {
    fprintf(out, "\n");
    fprintf(out, "  ┌─────────────────────────────────────────────────────────┐\n");
    fprintf(out, "  │                  sysctrace report                       │\n");
    fprintf(out, "  └─────────────────────────────────────────────────────────┘\n");
}

static void render_category_summary(FILE *out, const trace_context_t *tc) {
    fprintf(out, "\n  Category breakdown\n");
    fprintf(out, "  ──────────────────────────────────────────────\n");
    fprintf(out, "  %-10s %10s %14s %12s\n",
            "category", "calls", "total (us)", "avg (us)");
    fprintf(out, "  ──────────────────────────────────────────────\n");

    for (int c = 0; c < CAT_COUNT; c++) {
        const per_category_stats_t *s = &tc->per_category[c];
        if (s->count == 0) continue;
        double avg_us = ns_to_us(s->total_ns) / (double)s->count;
        fprintf(out, "  %-10s %10lu %14.1f %12.2f\n",
                category_name((category_t)c),
                (unsigned long)s->count,
                ns_to_us(s->total_ns),
                avg_us);
    }
    fprintf(out, "  ──────────────────────────────────────────────\n");
    double total_avg_us = tc->total_count
        ? ns_to_us(tc->total_ns) / (double)tc->total_count
        : 0.0;
    fprintf(out, "  %-10s %10lu %14.1f %12.2f\n",
            "TOTAL",
            (unsigned long)tc->total_count,
            ns_to_us(tc->total_ns),
            total_avg_us);
}

static void render_top_syscalls(FILE *out, const trace_context_t *tc, int top_n) {
    /* Collect non-empty syscall slots into an array we can sort. */
    row_t rows[TC_MAX_SYSCALL];
    int n = 0;
    for (long i = 0; i < TC_MAX_SYSCALL; i++) {
        if (tc->per_syscall[i].count > 0) {
            rows[n].nr = i;
            rows[n].s = tc->per_syscall[i];
            n++;
        }
    }

    qsort(rows, n, sizeof(row_t), cmp_by_count);
    int show = (n < top_n) ? n : top_n;

    fprintf(out, "\n  Top %d syscalls by frequency\n", show);
    fprintf(out, "  ──────────────────────────────────────────────────────────────\n");
    fprintf(out, "  %-16s %8s %12s %10s %10s %10s\n",
            "syscall", "calls", "total (us)", "avg (us)", "min (us)", "max (us)");
    fprintf(out, "  ──────────────────────────────────────────────────────────────\n");

    for (int i = 0; i < show; i++) {
        const char *name = syscall_name(rows[i].nr);
        char buf[32];
        if (!name) {
            snprintf(buf, sizeof(buf), "syscall_%ld", rows[i].nr);
            name = buf;
        }
        double avg_us = ns_to_us(rows[i].s.total_ns) / (double)rows[i].s.count;
        fprintf(out, "  %-16s %8lu %12.1f %10.2f %10.2f %10.2f\n",
                name,
                (unsigned long)rows[i].s.count,
                ns_to_us(rows[i].s.total_ns),
                avg_us,
                ns_to_us(rows[i].s.min_ns),
                ns_to_us(rows[i].s.max_ns));
    }
    fprintf(out, "  ──────────────────────────────────────────────────────────────\n");
}

void report_render(FILE *out, const trace_context_t *tc) {
    render_header(out);
    render_category_summary(out, tc);
    render_top_syscalls(out, tc, 10);

    /* Final wall-clock line. */
    if (tc->wall_end_ns > tc->wall_start_ns) {
        double wall_us = ns_to_us(tc->wall_end_ns - tc->wall_start_ns);
        fprintf(out, "\n  Wall-clock time: %.1f us  |  Sum syscall time: %.1f us  |  Ratio: %.1f%%\n",
                wall_us, ns_to_us(tc->total_ns),
                100.0 * (double)tc->total_ns / (double)(tc->wall_end_ns - tc->wall_start_ns));
    }
    fprintf(out, "\n");
}
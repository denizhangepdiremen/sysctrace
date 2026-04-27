/*
 * csv_writer.c — write syscall and category statistics as CSV
 */

#include "csv_writer.h"
#include "syscall_table.h"
#include "classifier.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

static int write_syscall_csv(const char *path, const trace_context_t *tc) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f, "syscall_nr,name,category,count,total_ns,avg_ns,min_ns,max_ns\n");

    for (long i = 0; i < TC_MAX_SYSCALL; i++) {
        const per_syscall_stats_t *s = &tc->per_syscall[i];
        if (s->count == 0) continue;

        const char *name = syscall_name(i);
        char fallback[24];
        if (!name) {
            snprintf(fallback, sizeof(fallback), "syscall_%ld", i);
            name = fallback;
        }

        category_t cat = classify(i);
        unsigned long avg_ns = (unsigned long)(s->total_ns / s->count);

        fprintf(f, "%ld,%s,%s,%lu,%lu,%lu,%lu,%lu\n",
                i, name, category_name(cat),
                (unsigned long)s->count,
                (unsigned long)s->total_ns,
                avg_ns,
                (unsigned long)s->min_ns,
                (unsigned long)s->max_ns);
    }

    fclose(f);
    return 0;
}

static int write_category_csv(const char *path, const trace_context_t *tc) {
    FILE *f = fopen(path, "w");
    if (!f) return -1;

    fprintf(f, "category,count,total_ns,avg_ns\n");

    for (int c = 0; c < CAT_COUNT; c++) {
        const per_category_stats_t *s = &tc->per_category[c];
        if (s->count == 0) continue;
        unsigned long avg_ns = (unsigned long)(s->total_ns / s->count);
        fprintf(f, "%s,%lu,%lu,%lu\n",
                category_name((category_t)c),
                (unsigned long)s->count,
                (unsigned long)s->total_ns,
                avg_ns);
    }

    fclose(f);
    return 0;
}

int csv_write(const char *prefix, const trace_context_t *tc) {
    char path[512];

    snprintf(path, sizeof(path), "%s_syscalls.csv", prefix);
    if (write_syscall_csv(path, tc) < 0) return -1;

    snprintf(path, sizeof(path), "%s_categories.csv", prefix);
    if (write_category_csv(path, tc) < 0) return -1;

    return 0;
}
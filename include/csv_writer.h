/*
 * csv_writer.h — emit per-syscall and per-category CSVs
 */
#ifndef CSV_WRITER_H
#define CSV_WRITER_H

#include "trace_context.h"

/*
 * Writes two files:
 *   <prefix>_syscalls.csv   - one row per syscall with stats
 *   <prefix>_categories.csv - one row per category
 *
 * Returns 0 on success, -1 on error (errno set).
 */
int csv_write(const char *prefix, const trace_context_t *tc);

#endif /* CSV_WRITER_H */
/*
 * reporter.h — render the trace summary report
 */
#ifndef REPORTER_H
#define REPORTER_H

#include "trace_context.h"
#include <stdio.h>

/* Print the full report to `out`. */
void report_render(FILE *out, const trace_context_t *tc);

#endif /* REPORTER_H */
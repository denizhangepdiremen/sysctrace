/*
 * tracer.h — ptrace-based syscall interception
 */
#ifndef TRACER_H
#define TRACER_H

#include <sys/types.h>
#include <stdint.h>

/* Statistics collected during a trace session */
typedef struct {
    uint64_t syscall_count;     /* total syscalls intercepted */
    uint64_t signal_count;      /* signals (other than SIGTRAP) seen */
    int      exit_status;       /* tracee's exit code */
} trace_stats_t;

/*
 * Fork-exec the given program under ptrace and run the intercept loop
 * until the tracee exits.
 *
 * argv:  NULL-terminated argv to exec (argv[0] is the program path)
 * stats: output, filled in by the function
 *
 * Returns 0 on success, -1 on error (sets errno).
 */
int tracer_run(char *const argv[], trace_stats_t *stats);

int tracer_run_v(char *const argv[], trace_stats_t *stats, int verbose);

/* Forward declaration — defined in trace_context.h */
struct trace_context;

/*
 * Full version: same as tracer_run_v but also fills the given
 * trace_context_t with per-syscall and per-category statistics.
 */
int tracer_run_full(char *const argv[], trace_stats_t *stats,
                    struct trace_context *tc, int verbose);

#endif /* TRACER_H */
/*
 * tracer.c — core ptrace loop
 *
 * Phase 4: timer + classifier + trace context.
 * Each syscall now records a precise latency, gets categorised, and
 * lands in the per-syscall and per-category aggregators.
 */

#include "tracer.h"
#include "regs_reader.h"
#include "syscall_table.h"
#include "trace_context.h"
#include "timer.h"
#include "classifier.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#define SYSCALL_STOP_SIG (SIGTRAP | 0x80)

static int run_child(char *const argv[]) {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        perror("[child] PTRACE_TRACEME");
        return -1;
    }
    execvp(argv[0], argv);
    perror("[child] execvp");
    return -1;
}

static void print_entry(uint64_t idx, const regs_snapshot_t *r) {
    const char *name = syscall_name((long)r->syscall_nr);
    if (name) {
        fprintf(stderr,
                "[%5lu] %-16s(0x%lx, 0x%lx, 0x%lx)",
                (unsigned long)idx, name,
                (unsigned long)r->arg0,
                (unsigned long)r->arg1,
                (unsigned long)r->arg2);
    } else {
        fprintf(stderr,
                "[%5lu] syscall_%-8lu(0x%lx, 0x%lx, 0x%lx)",
                (unsigned long)idx,
                (unsigned long)r->syscall_nr,
                (unsigned long)r->arg0,
                (unsigned long)r->arg1,
                (unsigned long)r->arg2);
    }
}

static void print_exit(int64_t retval, uint64_t latency_ns) {
    if (retval < 0 && retval >= -4095) {
        fprintf(stderr, " = -%ld   <%.2f us>\n", -retval, ns_to_us(latency_ns));
    } else if ((uint64_t)retval > 0xffff) {
        fprintf(stderr, " = 0x%lx   <%.2f us>\n", (unsigned long)retval, ns_to_us(latency_ns));
    } else {
        fprintf(stderr, " = %ld   <%.2f us>\n", (long)retval, ns_to_us(latency_ns));
    }
}

static int run_parent(pid_t child, trace_stats_t *stats,
                      trace_context_t *tc, int verbose) {
    int status;

    if (waitpid(child, &status, 0) < 0) {
        perror("[parent] initial waitpid");
        return -1;
    }
    if (!WIFSTOPPED(status)) {
        fprintf(stderr, "[parent] child did not stop as expected\n");
        return -1;
    }

    if (ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD) < 0) {
        perror("[parent] PTRACE_SETOPTIONS");
        return -1;
    }

    int      in_syscall   = 0;
    uint64_t entry_ts_ns  = 0;
    long     pending_nr   = 0;

    tc->wall_start_ns = timer_now_ns();

    for (;;) {
        if (ptrace(PTRACE_SYSCALL, child, 0, 0) < 0) {
            perror("[parent] PTRACE_SYSCALL");
            return -1;
        }

        if (waitpid(child, &status, 0) < 0) {
            perror("[parent] waitpid");
            return -1;
        }

        if (WIFEXITED(status)) {
            stats->exit_status = WEXITSTATUS(status);
            tc->wall_end_ns = timer_now_ns();
            return 0;
        }
        if (WIFSIGNALED(status)) {
            stats->exit_status = 128 + WTERMSIG(status);
            tc->wall_end_ns = timer_now_ns();
            return 0;
        }

        if (WIFSTOPPED(status)) {
            int sig = WSTOPSIG(status);

            if (sig == SYSCALL_STOP_SIG) {
                regs_snapshot_t r;
                if (regs_read(child, &r) < 0) {
                    perror("[parent] regs_read");
                    return -1;
                }

                if (!in_syscall) {
                    /* Entry stop. Record timestamp and syscall # for
                     * the matching exit stop. */
                    entry_ts_ns = timer_now_ns();
                    pending_nr  = (long)r.syscall_nr;
                    stats->syscall_count++;
                    if (verbose) {
                        print_entry(stats->syscall_count, &r);
                    }
                    in_syscall = 1;
                } else {
                    /* Exit stop. Latency = exit_ts - entry_ts. */
                    uint64_t exit_ts_ns = timer_now_ns();
                    uint64_t latency    = exit_ts_ns - entry_ts_ns;
                    tc_record(tc, pending_nr, latency);
                    if (verbose) {
                        print_exit(r.retval, latency);
                    }
                    in_syscall = 0;
                }
            } else {
                stats->signal_count++;
                if (ptrace(PTRACE_SYSCALL, child, 0, sig) < 0) {
                    perror("[parent] PTRACE_SYSCALL (signal forward)");
                    return -1;
                }
                if (waitpid(child, &status, 0) < 0) {
                    perror("[parent] waitpid (after signal)");
                    return -1;
                }
                if (WIFEXITED(status)) {
                    stats->exit_status = WEXITSTATUS(status);
                    tc->wall_end_ns = timer_now_ns();
                    return 0;
                }
                if (WIFSIGNALED(status)) {
                    stats->exit_status = 128 + WTERMSIG(status);
                    tc->wall_end_ns = timer_now_ns();
                    return 0;
                }
            }
        }
    }
}

int tracer_run(char *const argv[], trace_stats_t *stats) {
    return tracer_run_v(argv, stats, 0);
}

int tracer_run_v(char *const argv[], trace_stats_t *stats, int verbose) {
    /* Without a context, just discard the data. We allocate a
     * temporary one so the latency math still runs. */
    trace_context_t tc;
    tc_init(&tc);
    return tracer_run_full(argv, stats, &tc, verbose);
}

int tracer_run_full(char *const argv[], trace_stats_t *stats,
                    trace_context_t *tc, int verbose) {
    if (!argv || !argv[0] || !stats || !tc) {
        errno = EINVAL;
        return -1;
    }
    memset(stats, 0, sizeof(*stats));

    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        return -1;
    }
    if (child == 0) {
        _exit(run_child(argv) == 0 ? 0 : 127);
    }
    return run_parent(child, stats, tc, verbose);
}
/*
 * main.c — entry point for sysctrace
 * Phase 5: CSV export, --quiet flag, polished CLI
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sysctrace.h"
#include "tracer.h"
#include "trace_context.h"
#include "reporter.h"
#include "csv_writer.h"

static void print_banner(FILE *out) {
    fprintf(out, "┌─────────────────────────────────────────┐\n");
    fprintf(out, "│  sysctrace v%s                       │\n", SYSCTRACE_VERSION);
    fprintf(out, "│  Lightweight Linux Syscall Tracer       │\n");
    fprintf(out, "└─────────────────────────────────────────┘\n");
}

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s [options] -- <program> [args...]\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h, --help          show this help\n");
    fprintf(stderr, "  -v, --version       show version\n");
    fprintf(stderr, "  -V, --verbose       print every syscall to stderr\n");
    fprintf(stderr, "  -q, --quiet         suppress the summary report\n");
    fprintf(stderr, "  -c, --csv <prefix>  write <prefix>_syscalls.csv and <prefix>_categories.csv\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s -- ls -la /tmp\n", prog);
    fprintf(stderr, "  %s -V -- curl -s example.com\n", prog);
    fprintf(stderr, "  %s --csv out -- cp /etc/hosts /tmp/hosts.bak\n", prog);
}

typedef struct {
    int          verbose;
    int          quiet;
    const char  *csv_prefix;
    int          sep;
} cli_opts_t;

static int parse_opts(int argc, char **argv, cli_opts_t *opts) {
    opts->verbose    = 0;
    opts->quiet      = 0;
    opts->csv_prefix = NULL;
    opts->sep        = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            opts->sep = i;
            return 0;
        }
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0) {
            opts->verbose = 1;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            opts->quiet = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--csv") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "[error] %s requires an argument\n", argv[i]);
                return -1;
            }
            opts->csv_prefix = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_banner(stdout);
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("sysctrace %s\n", SYSCTRACE_VERSION);
            exit(EXIT_SUCCESS);
        } else {
            fprintf(stderr, "[error] unknown option: %s\n", argv[i]);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_banner(stdout);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    cli_opts_t opts;
    if (parse_opts(argc, argv, &opts) < 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (opts.sep < 0 || opts.sep == argc - 1) {
        fprintf(stderr, "[error] missing '--' separator or no program given\n\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char **target_argv = &argv[opts.sep + 1];

    if (!opts.quiet) {
        print_banner(stderr);
        fprintf(stderr, "[sysctrace] tracing: %s", target_argv[0]);
        for (int i = 1; target_argv[i] != NULL; i++) {
            fprintf(stderr, " %s", target_argv[i]);
        }
        fprintf(stderr, "\n[sysctrace] ----------------------------------------\n");
    }

    trace_context_t tc;
    tc_init(&tc);

    trace_stats_t stats;
    if (tracer_run_full(target_argv, &stats, &tc, opts.verbose) < 0) {
        fprintf(stderr, "[error] tracer_run failed\n");
        return EXIT_FAILURE;
    }

    if (!opts.quiet) {
        fprintf(stderr, "[sysctrace] ----------------------------------------\n");
        fprintf(stderr, "[sysctrace] tracee exited with status %d\n", stats.exit_status);
        fprintf(stderr, "[sysctrace] syscalls intercepted : %lu\n",
                (unsigned long)stats.syscall_count);
        fprintf(stderr, "[sysctrace] signals forwarded    : %lu\n",
                (unsigned long)stats.signal_count);
        report_render(stderr, &tc);
    }

    if (opts.csv_prefix) {
        if (csv_write(opts.csv_prefix, &tc) < 0) {
            perror("[error] csv_write");
            return EXIT_FAILURE;
        }
        fprintf(stderr, "[sysctrace] CSV written: %s_syscalls.csv, %s_categories.csv\n",
                opts.csv_prefix, opts.csv_prefix);
    }

    return stats.exit_status;
}
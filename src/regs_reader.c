/*
 * regs_reader.c — x86-64 register access via ptrace
 *
 * x86-64 Linux syscall calling convention:
 *   syscall #   : rax
 *   args 0..5   : rdi, rsi, rdx, r10, r8, r9
 *   return val  : rax (after syscall returns)
 *
 * On a syscall entry stop, rax holds the syscall number.
 * On a syscall exit stop, rax holds the return value (a signed long;
 * negative values in [-4095, -1] are -errno).
 */

#include "regs_reader.h"

#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>

int regs_read(pid_t tracee, regs_snapshot_t *out) {
    if (!out) {
        errno = EINVAL;
        return -1;
    }

    struct user_regs_struct r;
    if (ptrace(PTRACE_GETREGS, tracee, 0, &r) < 0) {
        return -1;
    }

    /* Map x86-64 register names to our portable struct.
     * The struct user_regs_struct field names are GNU/Linux
     * x86-64 specific. */
    out->syscall_nr = r.orig_rax;  /* orig_rax preserves the
                                      syscall # even at exit, when
                                      rax has been overwritten by
                                      the return value. */
    out->arg0   = r.rdi;
    out->arg1   = r.rsi;
    out->arg2   = r.rdx;
    out->arg3   = r.r10;
    out->arg4   = r.r8;
    out->arg5   = r.r9;
    out->retval = (int64_t)r.rax;

    return 0;
}
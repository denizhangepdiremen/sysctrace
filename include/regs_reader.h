/*
 * regs_reader.h — x86-64 register snapshot via ptrace
 */
#ifndef REGS_READER_H
#define REGS_READER_H

#include <sys/types.h>
#include <stdint.h>

/*
 * Architecture-neutral wrapper around the registers we care about.
 * Keeping this struct separate from struct user_regs_struct lets us
 * port the rest of the code if we ever target RISC-V.
 */
typedef struct {
    uint64_t syscall_nr;   /* rax on entry */
    uint64_t arg0;         /* rdi */
    uint64_t arg1;         /* rsi */
    uint64_t arg2;         /* rdx */
    uint64_t arg3;         /* r10 */
    uint64_t arg4;         /* r8  */
    uint64_t arg5;         /* r9  */
    int64_t  retval;       /* rax on exit */
} regs_snapshot_t;

/*
 * Read the tracee's registers via PTRACE_GETREGS and fill `out`.
 * Returns 0 on success, -1 on error (errno set by ptrace).
 */
int regs_read(pid_t tracee, regs_snapshot_t *out);

#endif /* REGS_READER_H */
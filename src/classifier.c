/*
 * classifier.c — syscall number → semantic category
 *
 * The mapping is hand-curated for common x86-64 Linux syscalls.
 * It does not need to be exhaustive: anything not listed below
 * falls into CAT_OTHER, which is fine for our analysis.
 */

#include "classifier.h"

#include <stddef.h>

/* Convenience macros for readability. */
#define IO   CAT_IO
#define MEM  CAT_MEMORY
#define NET  CAT_NETWORK
#define PROC CAT_PROCESS
#define OTH  CAT_OTHER

/* Direct-indexed table, mirroring syscall_table.c.
 * Default value (0) happens to be CAT_IO, so we explicitly
 * set every known entry and use a sentinel for "unset". */
#define MAX_SYSCALL 335

/* We use a parallel "known" bitmap to distinguish CAT_IO entries
 * from "default 0" entries, but a simpler approach: initialise
 * everything to CAT_OTHER, then override known ones. */
static category_t table[MAX_SYSCALL];
static int initialized = 0;

static void init_table(void) {
    for (int i = 0; i < MAX_SYSCALL; i++) table[i] = CAT_OTHER;

    /* I/O */
    table[0]   = IO;   /* read       */
    table[1]   = IO;   /* write      */
    table[2]   = IO;   /* open       */
    table[3]   = IO;   /* close      */
    table[4]   = IO;   /* stat       */
    table[5]   = IO;   /* fstat      */
    table[6]   = IO;   /* lstat      */
    table[7]   = IO;   /* poll       */
    table[8]   = IO;   /* lseek      */
    table[16]  = IO;   /* ioctl      */
    table[17]  = IO;   /* pread64    */
    table[18]  = IO;   /* pwrite64   */
    table[19]  = IO;   /* readv      */
    table[20]  = IO;   /* writev     */
    table[21]  = IO;   /* access     */
    table[22]  = IO;   /* pipe       */
    table[23]  = IO;   /* select     */
    table[32]  = IO;   /* dup        */
    table[33]  = IO;   /* dup2       */
    table[72]  = IO;   /* fcntl      */
    table[73]  = IO;   /* flock      */
    table[74]  = IO;   /* fsync      */
    table[78]  = IO;   /* getdents   */
    table[79]  = IO;   /* getcwd     */
    table[80]  = IO;   /* chdir      */
    table[82]  = IO;   /* rename     */
    table[83]  = IO;   /* mkdir      */
    table[84]  = IO;   /* rmdir      */
    table[85]  = IO;   /* creat      */
    table[86]  = IO;   /* link       */
    table[87]  = IO;   /* unlink     */
    table[88]  = IO;   /* symlink    */
    table[89]  = IO;   /* readlink   */
    table[90]  = IO;   /* chmod      */
    table[92]  = IO;   /* chown      */
    table[217] = IO;   /* getdents64 */
    table[257] = IO;   /* openat     */
    table[258] = IO;   /* mkdirat    */
    table[259] = IO;   /* mknodat    */
    table[260] = IO;   /* fchownat   */
    table[262] = IO;   /* newfstatat */
    table[263] = IO;   /* unlinkat   */
    table[264] = IO;   /* renameat   */
    table[267] = IO;   /* readlinkat */
    table[268] = IO;   /* fchmodat   */
    table[269] = IO;   /* faccessat  */
    table[292] = IO;   /* dup3       */
    table[293] = IO;   /* pipe2      */
    table[332] = IO;   /* statx      */

    /* Memory */
    table[9]   = MEM;  /* mmap       */
    table[10]  = MEM;  /* mprotect   */
    table[11]  = MEM;  /* munmap     */
    table[12]  = MEM;  /* brk        */
    table[25]  = MEM;  /* mremap     */
    table[28]  = MEM;  /* madvise    */
    table[319] = MEM;  /* memfd_create */

    /* Network */
    table[41]  = NET;  /* socket      */
    table[42]  = NET;  /* connect     */
    table[43]  = NET;  /* accept      */
    table[44]  = NET;  /* sendto      */
    table[45]  = NET;  /* recvfrom    */
    table[46]  = NET;  /* sendmsg     */
    table[47]  = NET;  /* recvmsg     */
    table[48]  = NET;  /* shutdown    */
    table[49]  = NET;  /* bind        */
    table[50]  = NET;  /* listen      */
    table[51]  = NET;  /* getsockname */
    table[52]  = NET;  /* getpeername */
    table[54]  = NET;  /* setsockopt  */
    table[55]  = NET;  /* getsockopt  */
    table[288] = NET;  /* accept4     */

    /* Process */
    table[56]  = PROC; /* clone       */
    table[57]  = PROC; /* fork        */
    table[58]  = PROC; /* vfork       */
    table[59]  = PROC; /* execve      */
    table[60]  = PROC; /* exit        */
    table[61]  = PROC; /* wait4       */
    table[62]  = PROC; /* kill        */
    table[231] = PROC; /* exit_group  */

    initialized = 1;
}

category_t classify(long syscall_nr) {
    if (!initialized) init_table();
    if (syscall_nr < 0 || syscall_nr >= MAX_SYSCALL) return CAT_OTHER;
    return table[syscall_nr];
}

const char *category_name(category_t c) {
    switch (c) {
        case CAT_IO:      return "I/O";
        case CAT_MEMORY:  return "Memory";
        case CAT_NETWORK: return "Network";
        case CAT_PROCESS: return "Process";
        case CAT_OTHER:   return "Other";
        default:          return "?";
    }
}
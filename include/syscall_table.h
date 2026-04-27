/*
 * syscall_table.h — x86-64 Linux syscall number → name mapping
 */
#ifndef SYSCALL_TABLE_H
#define SYSCALL_TABLE_H

#include <stdint.h>

/*
 * Returns a static, NUL-terminated string with the syscall name
 * for the given x86-64 syscall number, or NULL if unknown.
 * The returned pointer is valid for the lifetime of the program.
 */
const char *syscall_name(long nr);

#endif /* SYSCALL_TABLE_H */
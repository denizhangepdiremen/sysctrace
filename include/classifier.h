/*
 * classifier.h — map a syscall number to a semantic category.
 */
#ifndef CLASSIFIER_H
#define CLASSIFIER_H

typedef enum {
    CAT_IO = 0,         /* read, write, open, close, ...        */
    CAT_MEMORY,         /* mmap, munmap, brk, mprotect, ...     */
    CAT_NETWORK,        /* socket, connect, send, recv, ...     */
    CAT_PROCESS,        /* fork, execve, wait, clone, exit, ... */
    CAT_OTHER,          /* everything else                      */
    CAT_COUNT
} category_t;

/*
 * Returns the category for the given x86-64 syscall number.
 * Unknown numbers fall into CAT_OTHER.
 */
category_t classify(long syscall_nr);

/* Human-readable name for a category. Returned pointer is static. */
const char *category_name(category_t c);

#endif /* CLASSIFIER_H */
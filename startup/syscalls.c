/**
 * @file  syscalls.c
 * @brief Minimal newlib system call stubs (heap via _sbrk). _write lives in uart.c.
 */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

extern uint8_t _end;          /* end of .bss (linker) */
extern uint8_t _estack;       /* top of RAM (linker) */
extern uint32_t _Min_Stack_Size;

static uint8_t *heap_end;

void *_sbrk(ptrdiff_t incr)
{
    const uint8_t *limit = &_estack - (uint32_t)&_Min_Stack_Size;
    if (heap_end == NULL) heap_end = &_end;
    if (heap_end + incr > limit) {
        errno = ENOMEM;
        return (void *)-1;
    }
    uint8_t *prev = heap_end;
    heap_end += incr;
    return prev;
}

int _close(int fd)                 { (void)fd; return -1; }
int _fstat(int fd, struct stat *s) { (void)fd; s->st_mode = S_IFCHR; return 0; }
int _isatty(int fd)                { (void)fd; return 1; }
int _lseek(int fd, int p, int d)   { (void)fd; (void)p; (void)d; return 0; }
int _read(int fd, char *p, int n)  { (void)fd; (void)p; (void)n; return 0; }
int _getpid(void)                  { return 1; }
int _kill(int pid, int sig)        { (void)pid; (void)sig; errno = EINVAL; return -1; }
void _exit(int status)             { (void)status; for (;;) { } }

#ifndef MELOX_STD_H
#define MELOX_STD_H
#include <syscall/syscall.h>
#include <stdint.h>

typedef int32_t pid_t;

__MXSYSCALL(pid_t, fork)

// __MXSYSCALL1(int, sbrk, void*, addr)

// __MXSYSCALL1(void*, brk, unsigned long, size)

__MXSYSCALL(pid_t, getpid)

__MXSYSCALL(pid_t, getppid)

__MXSYSCALL1(void, _exit, int, status)

// __MXSYSCALL1(unsigned int, sleep, unsigned int, seconds)

//__MXSYSCALL1(pid_t, wait, int*, status);

#endif
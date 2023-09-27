#ifndef MELOX_SYSCALL_H
#define MELOX_SYSCALL_H

#define __SYSCALL_fork 1
// #define __SYSCALL_yield 2
// #define __SYSCALL_sbrk 3
// #define __SYSCALL_brk 4
#define __SYSCALL_getpid 2
#define __SYSCALL_getppid 3
// #define __SYSCALL_sleep 7
#define __SYSCALL__exit 4
//#define __SYSCALL_wait 9

#define __SYSCALL_MAX 0x100
#ifndef __ASM__

void
syscall_install();

static void*
syscall(unsigned int callcode)
{
    asm volatile("int %0" ::"i"(0x80), "D"(callcode) : "eax");
}

#define asmlinkage __attribute__((regparm(0)))

#define __PARAM_MAP1(t1, p1) t1 p1
#define __PARAM_MAP2(t1, p1, ...) t1 p1, __PARAM_MAP1(__VA_ARGS__)
#define __PARAM_MAP3(t1, p1, ...) t1 p1, __PARAM_MAP2(__VA_ARGS__)
#define __PARAM_MAP4(t1, p1, ...) t1 p1, __PARAM_MAP3(__VA_ARGS__)
#define __PARAM_MAP5(t1, p1, ...) t1 p1, __PARAM_MAP4(__VA_ARGS__)
#define __PARAM_MAP6(t1, p1, ...) t1 p1, __PARAM_MAP5(__VA_ARGS__)

#define ___DOINT80(callcode, rettype)                                          \
    int v;                                                                     \
    asm volatile("int %1\n" : "=a"(v) : "i"(0x80), "a"(callcode));  \
    return (rettype)v;

#define __DEFINE_MXSYSCALL(rettype, name) asmlinkage rettype __mxsys_##name()

#define __DEFINE_MXSYSCALL1(rettype, name, t1, p1)                             \
    asmlinkage rettype __mxsys_##name(__PARAM_MAP1(t1, p1))

#define __DEFINE_MXSYSCALL2(rettype, name, t1, p1, t2, p2)                     \
    asmlinkage rettype __mxsys_##name(__PARAM_MAP2(t1, p1, t2, p2))

#define __DEFINE_MXSYSCALL3(rettype, name, t1, p1, t2, p2, t3, p3)             \
    asmlinkage rettype __mxsys_##name(__PARAM_MAP3(t1, p1, t2, p2, t3, p3));

#define __DEFINE_MXSYSCALL4(rettype, name, t1, p1, t2, p2, t3, p3, t4, p4)     \
    asmlinkage rettype __mxsys_##nam(                                          \
      __PARAM_MAP4(t1, p1, t2, p2, t3, p3, t4, p4))

#define __MXSYSCALL(rettype, name)                                             \
    static rettype name()                                                      \
    {                                                                          \
        ___DOINT80(__SYSCALL_##name, rettype)                                  \
    }

#define __MXSYSCALL1(rettype, name, t1, p1)                                    \
    static rettype name(__PARAM_MAP1(t1, p1))                                  \
    {                                                                          \
        asm("" ::"b"(p1));                                                     \
        ___DOINT80(__SYSCALL_##name, rettype)                                  \
    }

#define __MXSYSCALL2(rettype, name, t1, p1, t2, p2)                            \
    static rettype name(__PARAM_MAP2(t1, p1, t2, p2))                          \
    {                                                                          \
        asm("\n" ::"b"(p1), "c"(p2));                                          \
        ___DOINT80(__SYSCALL_##name, rettype)                                  \
    }

#define __MXSYSCALL3(rettype, name, t1, p1, t2, p2, t3, p3)                    \
    static rettype name(__PARAM_MAP3(t1, p1, t2, p2, t3, p3))                  \
    {                                                                          \
        asm("\n" ::"b"(p1), "c"(p2), "d"(p3));                                 \
        ___DOINT80(__SYSCALL_##name, rettype)                                  \
    }

#define __MXSYSCALL4(rettype, name, t1, p1, t2, p2, t3, p3, t4, p4)            \
    static rettype name(__PARAM_MAP3(t1, p1, t2, p2, t3, p3, t4, p4))          \
    {                                                                          \
        asm("\n" ::"b"(p1), "c"(p2), "d"(p3), "D"(p4));                        \
        ___DOINT80(__SYSCALL_##name, rettype)                                  \
    }

#endif
#endif
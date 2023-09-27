#include <libs/mstdio.h>
#include <libs/mstdlib.h>
#include <stdint.h>
#include <kernel/process/sched.h>
#include <kernel/cpu/cpu.h>
#include <kernel/time/time.h>
#include <syscall/syscall.h>
#include <syscall/meloxstd.h>

void init_platform();
extern void _mxinit_main(); /* mxinit.c */
void __proc0()
{
    init_platform();
    kprintf("This is proc0!");
    if (!fork()) {
        kprintf("fork sucess!");
        asm("jmp _mxinit_main");
    }

    asm("1: jmp 1b");
}

void init_platform()
{
    timer_init();
    init_keyboard();

    syscall_install();
}
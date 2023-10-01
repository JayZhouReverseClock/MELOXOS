#include <libs/mstdio.h>
#include <libs/mstdlib.h>
#include <stdint.h>
#include <kernel/process/sched.h>
#include <kernel/cpu/cpu.h>
#include <kernel/time/time.h>
#include <syscall/syscall.h>
#include <syscall/meloxstd.h>
extern void __init_phykernel_end;
void init_platform();
extern void _mxinit_main(); /* mxinit.c */
void __proc0()
{
    init_platform();
    kprintf("This is proc0!\n");
    if (1 == fork()) {
        schedule();
        kprintf("fork sucess!\n");
        asm("jmp _mxinit_main");
    }

    asm("1: jmp 1b");
}

void init_platform()
{
    //size_t virk_init_pg_count = ((uintptr_t)(&__init_phykernel_end)) >> 12;
    //init malloc
    mem_init();
    timer_init();
    init_keyboard();

    syscall_install();
    // 清除 hhk_init 与前1MiB的映射
    // size_t virk_init_pg_count = ((uintptr_t)(&__init_phykernel_end)) >> 12;
    // for (size_t i = 0; i < virk_init_pg_count; i++) {

    //     vmm_unmap_page(KERNEL_PID, (i << 12));
    // }
    // kprintf("[MM] Releaseing %d pages from 0x0.\n", virk_init_pg_count);
}
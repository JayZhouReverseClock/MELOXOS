#include <libs/mstdio.h>
#include <libs/mstdlib.h>
#include <stdint.h>
#include <kernel/process/sched.h>
#include <kernel/cpu/cpu.h>
#include <kernel/time/time.h>
#include <syscall/syscall.h>
#include <syscall/meloxstd.h>
#include <kernel/cpu/io.h>
#include <kernel/cpu/pci.h>
#include <blk_drive/ata_dev.h>
extern void __init_phykernel_end;
void init_platform();
extern void _mxinit_main(); /* mxinit.c */
void __proc0()
{
    init_platform();
    //kprintf("This is proc0!\n");
    if (!fork()) {
        //schedule();
        //kprintf("fork sucess!\n");
        asm("jmp _mxinit_main");
    }
    kprintf("proc 0");
    //io_outb(0x20, 0x20);
    //io_outb(0xa0, 0x20);//EOI end the int
    asm("1: jmp 1b");
}

void init_platform()
{
    //size_t virk_init_pg_count = ((uintptr_t)(&__init_phykernel_end)) >> 12;
    //init malloc
    mem_init();
    init_keyboard();
    timer_init();
    pci_init();
    pci_print_device();
    ide_init();
    syscall_install();
    // 清除 hhk_init 与前1MiB的映射
    // size_t virk_init_pg_count = ((uintptr_t)(&__init_phykernel_end)) >> 12;
    // for (size_t i = 0; i < virk_init_pg_count; i++) {

    //     vmm_unmap_page(KERNEL_PID, (i << 12));
    // }
    // kprintf("[MM] Releaseing %d pages from 0x0.\n", virk_init_pg_count);
}
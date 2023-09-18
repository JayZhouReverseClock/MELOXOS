#include <init/multiboot.h>
#include <libs/mstdio.h>
#include <libs/mstdlib.h>
#include <vga/vga.h>
#include <init/gdt.h>
#include <init/idt.h>
#include <stdint.h>
#include <kernel/memory/page.h>
#include <kernel/memory/phy_mem.h>
#include <kernel/memory/vir_mem.h>
#include <kernel/memory/malloc.h>
#include <libs/mstdio.h>
#include <kernel/cpu/cpu.h>
/* Macros. */

/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

/* Some screen stuff. */
/* The number of columns. */
#define COLUMNS                 80
/* The number of lines. */
#define LINES                   24
/* The attribute of an character. */
#define ATTRIBUTE               7
/* The video memory address. */
#define VIDEO                   0xB8000UL
#define VGA_BUFFER_SIZE         4096
#define VGA_BUFFER_VADDR        0xB0000000UL

extern void virkernel_start;
extern void virkernel_end ;
extern void __init_phykernel_end;

/* Forward declarations. */
void _kernel_init (unsigned long addr);

void _kernel_init(unsigned long addr)
{
    multiboot_info_t *mbi;
    mbi = (multiboot_info_t *) addr;
    multiboot_memory_map_t* map = (multiboot_memory_map_t*)mbi->mmap_addr;
    _init_idt();
    pmm_init(MEM_1MB + (mbi->mem_upper << 10));
    vmm_init();

    init_vga_buffer();
    vga_set_show(0,VGA_COLOR_BLACK,VGA_COLOR_LIGHT_WHITE);
    /* Clear the screen. */
    vga_clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    // if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    // {
    //   kprintf("Invalid magic number: 0x%x\n", (unsigned) magic);
    //   return;
    // }

    /* Set MBI to the address of the Multiboot information structure. */

    /* Print out the flags. */
    kprintf("flags = 0x%x\n", (unsigned) mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        kprintf("mem_lower = %uKB, mem_upper = %uKB\n",(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);
    
    unsigned int map_size = mbi->mmap_length / sizeof(multiboot_memory_map_t);
    for (unsigned int i = 0; i < map_size; i++) {
        multiboot_memory_map_t mmap = map[i];
        kprintf("[MM] Base: 0x%x, len: %u KiB, type: %u\n",
               map[i].addr_low,
               map[i].len_low >> 10,
               map[i].type);
        if (mmap.type == MULTIBOOT_MEMORY_AVAILABLE) {
            // 整数向上取整除法
            uintptr_t pg = map[i].addr_low + 0x0fffU;
            pmm_mark_chunk_free(pg >> 12, map[i].len_low >> 12);
            kprintf("[MM] Freed %u pages start from 0x%x\n",
                   map[i].len_low >> 12,
                   pg & ~0x0fffU);
        }
    }

    // 将内核占据的页设为已占用
    size_t pg_count = (uintptr_t)(&virkernel_end  - &virkernel_start) >> 12;
    pmm_mark_chunk_occupied(V2P(&virkernel_start) >> 12, pg_count);
    kprintf("[MM] Allocated %d pages for kernel.\n", pg_count);

    size_t vga_buf_pgs = VGA_BUFFER_SIZE >> 12;
    
    // 首先，标记VGA部分为已占用
    pmm_mark_chunk_occupied(VIDEO >> 12, vga_buf_pgs);
    
    // 重映射VGA文本缓冲区（以后会变成显存，i.e., framebuffer）
    for (size_t i = 0; i < vga_buf_pgs; i++)
    {
        vmm_map_page(VGA_BUFFER_VADDR + (i << 12), VIDEO + (i << 12), PG_PREM_RW, PG_PREM_RW);
    }
    set_vga_buffer(VGA_BUFFER_VADDR);
    kprintf("[MM] Mapped VGA to %x.\n", VGA_BUFFER_VADDR);

    // 为内核创建一个专属栈空间。
    for (size_t i = 0; i < (K_STACK_SIZE >> 12); i++) {
        vmm_alloc_page(K_STACK_START + (i << 12), PG_PREM_RW, PG_PREM_RW);
    }
    kprintf("[MM] Allocated %d pages for stack start at %p\n", K_STACK_SIZE>>12, K_STACK_START);

    //give malloc init
    mem_init();
    kprintf("[MM] Malloc Init Success \n");
    kprintf("[KERNEL] === Initialization Done === \n\n");

    vga_put_str("MELOX OS\n");
     //__asm__("int $1");
}

void _kernel_finnal_init() {
    //kprintf("[KERNEL] === Post Initialization === \n");
    size_t virk_init_pg_count = ((uintptr_t)(&__init_phykernel_end)) >> 12;
    kprintf("[MM] Releaseing %d pages from 0x0.\n", virk_init_pg_count);
    
    // // 清除 hhk_init 与前1MiB的映射
    for (size_t i = 0; i < virk_init_pg_count; i++) {

        vmm_unmap_page((i << 12));
    }

    // // 清除 hhk_init 与前1MiB的映射
    // for (size_t i = 0; i < 256; i++) {

    //     vmm_unmap_page((i << 12));
    // }
    // for (size_t i = 256; i < virk_init_pg_count; i++) {

    //     vmm_unmap_page((i << 12));
    // }
    kprintf("[KERNEL] === Post Initialization Done === \n\n");
}

void _kernel_main()
{
    char* buf[64];
    
    kprintf("We are now running in virtual address space!\n\n");
    
    cpu_get_brand(buf);
    kprintf("CPU: %s\n\n", buf);

    uintptr_t k_start = vmm_v2p(&virkernel_start);
    kprintf("The kernel's base address mapping: %x->%x\n", &virkernel_start, k_start);
    //__asm__("int $0");
    // test malloc & free

    uint8_t* big_ = k_malloc(1000);
    big_[0] = 123;
    big_[1] = 23;
    big_[2] = 3;

    kprintf("malloc %d, %d, %d\n", big_[0], big_[1], big_[2]);
    malloc_free(big_);

    init_keyboard();
}
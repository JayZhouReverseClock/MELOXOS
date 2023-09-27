#define ASM_FILE        1
#include <init/multiboot.h>
#define STACK_SIZE                      0x4000
#define MB_INFO_SPACE_SIZE 0x1000
/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#define MULTIBOOT_HEADER_FLAGS   MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
#define KPG_SIZE    6*4*1024        

.section .multiboot_header
    .align 4
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_HEADER_FLAGS 
    .long CHECKSUM(MULTIBOOT_HEADER_FLAGS)

.section .bss
    /* According to System V ABI, the stack must be aligned at 16 bytes boundary */
    .global mb_info
    .align 16
    mb_info://this is mulyiboot_info space,we save it for usr when vir addr
        .skip MB_INFO_SPACE_SIZE
    stack_bottom:
        .skip STACK_SIZE, 0
    stack_top:

/* 
    1 page directory, 
    5 page tables:
        1. Mapping reserved area and hhk_init
        2-5. Remapping the kernels
*/

.section .kernerl_page
    .global _kernel_pt
    _kernel_pt:
        .skip KPG_SIZE, 0
// now we need to trans to virtue address,so we need to fo this
// first is save the multiboot_info, caus the ebx save the multiboot_info address
// when we begin to virture address, the ebx address will lost destinition(ebx save addr is phy addr, but we are int vir addr)
// so we need to save the multiboot_info in phyaddr which we can find,then we can use it when we begin to vir addr

.section .virk_init
    .global start
    .type start, @function

    start:
        cli
        movl $stack_top, %esp
        pushl $mb_info
        pushl %ebx
        call save_multiboot_info
        popl %eax
        popl %eax
    //now we start to identity mapping kernel to 3GB
    //begin to page, so we need to map page
        pushl $KPG_SIZE
        pushl $(_kernel_pt - 0xC0000000)
        call kerner_init
    
    //now we begin to start page
    //1. movl the pt address to cr3

        movl (%esp), %eax
        andl $0xfffff000, %eax
        movl %eax, %cr3

    //2. start page,(CR0.PG=1)
        movl %cr0, %eax
        orl $0x80010000, %eax
        movl %eax, %cr0

    //now we are in virture mem, go to virkeenel
        popl %eax
        popl %eax
        pushl $_kernel_start
        ret



/*
.section .multiboot_entry
    .global start
    .type start, @function

    start:
        movl $stack_top, %esp
        call _init_gdt
        call _init_idt

        pushl $_gdt
        pushw _gdt_limit
        lgdt (%esp)

        movl $_idt, 2(%esp)
        movw _idt_limit, %ax
        movw %ax, (%esp)
        lidt (%esp)
        popw %ax
        popl %eax


        movw $0x10, %cx
        movw %cx, %es
        movw %cx, %ds
        movw %cx, %fs
        movw %cx, %gs
        movw %cx, %ss

        pushw $0x08
        pushl $_after_gdt
        retf

    _after_gdt:
        pushl %ebx
        pushl %eax
        call _kernel_init

    loop:   hlt
        jmp     loop
*/





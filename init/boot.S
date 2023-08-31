#define ASM_FILE        1
#include <init/multiboot.h>
#define STACK_SIZE                      0x4000
/* C symbol format. HAVE_ASM_USCORE is defined by configure. */
#define MULTIBOOT_HEADER_FLAGS   MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
        

.section .multiboot_header
    .align 4
    .long MULTIBOOT_MAGIC
    .long MULTIBOOT_HEADER_FLAGS 
    .long CHECKSUM(MULTIBOOT_HEADER_FLAGS)

.section .bss
    /* According to System V ABI, the stack must be aligned at 16 bytes boundary */
    .align 16
    stack_bottom:
        .skip STACK_SIZE, 0
    stack_top:


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






// this is new virture kernel entry

.section .text
    .global _kernel_start
    _kernel_start:

        call _init_gdt
        pushl $_gdt
        pushw _gdt_limit
        lgdt (%esp)

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
        movl $mb_info, (%esp)
        call _kernel_init

         /* 
            load new stack point, the 0xffc00000 - 0xffffffff is
            used by page self map,so stack is begin with 0xffbffff0
        */
        mov $0xffbffff0, %esp
        call _kernel_finnal_init

        subl $6, %esp
        movl $_idt, 2(%esp)
        movw _idt_limit, %ax
        movw %ax, (%esp)
        lidt (%esp)
        addl $6, %esp

        call _kernel_main

        cli
    j_:
        hlt
        jmp j_
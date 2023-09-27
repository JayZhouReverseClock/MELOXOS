#define __ASM__
#include <kernel/interrupt/int_vector.h>

.section .data
    /*
        注意，这里的顺序非常重要。每个系统调用在这个地址表里的索引等于其调用号。
    */
    syscall_table:
        1:
        .long 0
        .long __mxsys_fork          /* 1 */
        //.long __mxsys_yield
        //.long __mxsys_sbrk
        //.long __mxsys_brk
        .long __mxsys_getpid        /* 5 */
        .long __mxsys_getppid
        //.long __mxsys_sleep
        .long __mxsys_exit
        //.long __mxsys_wait          /* 9 */
        2:
        .rept 0x100 - (2b - 1b)/4
            .long 0
        .endr

.global syscall_handlr

.section .text
    syscall_handlr:
        pushl %ebp
        movl 8(%esp), %ebp

        movl  (%ebp), %eax          /* eax: call code as well as the return value from syscall */
        cmpl  $0x100, %eax
        jae 2f

        shll $2, %eax
        addl $syscall_table, %eax
        cmpl $0, (%eax)
        jne 1f
    2:    
        neg   %eax
        popl  %ebp
        ret
    1:
        pushl 24(%ebp)      /* esi - #6 arg */
        pushl 20(%ebp)      /* ebp - #5 arg */
        pushl 16(%ebp)      /* edi - #4 arg */
        pushl 12(%ebp)      /* edx - #3 arg */
        pushl 8(%ebp)       /* ecx - #2 arg */
        pushl 4(%ebp)       /* ebx - #1 arg */
        
        call (%eax)

        movl %eax, (%ebp)    /* save the return value */

        addl $24, %esp      /* remove the parameters from stack */

        popl %ebp
        
        ret



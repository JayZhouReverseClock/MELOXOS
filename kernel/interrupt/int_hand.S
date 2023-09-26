/*
 *  melox/kernel/int_hand.S
 *
 *  (C) 2023  LHL(MELODY)
 */

/*
this is a interrupt hand file
stack is
no_err_stack:
cs
ip
interruput hand function
edx
ebp
ss
ds
es
fs
errcode
->ip

err_code_stack:
cs
ip
err_code
function
edx
ebp
ss
ds
es
fs
errcode
->ip

 */

.globl divide_error,debug,nmi,int3,overflow,bounds,invalid_op
.globl double_fault,coprocessor_segment_overrun
.globl invalid_TSS,segment_not_present,stack_segment
.globl general_protection,coprocessor_error,irq13,reserved,handle_keywords
.globl intr_time_handler

 _no_err_code:
 	xchgl %eax,(%esp)
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl $0		# "error code"
	lea 44(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%eax
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

/*	xchgl %eax,(%esp)
	pushl %ebx
	pushl %edx
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl $0		# "error code"
	lea 32(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%eax
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %edx
	popl %ebx
	popl %eax
	iret
*/

_err_code:
	xchgl %eax,4(%esp)		# error code <-> %eax
	xchgl %ebx,(%esp)		# &function <-> %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl %eax			# error code
	lea 44(%esp),%eax		# offset
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	call *%ebx
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret
	/*
	xchgl %eax,4(%esp)		# error code <-> %eax
	xchgl %ebx,(%esp)		# &function <-> %ebx
	pushl %edx
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl %eax			# error code
	lea 32(%esp),%eax		# offset
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	call *%ebx
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %edx
	popl %ebx
	popl %eax
	iret
	*/

intr_time_handler:
	pushl %esp

	pushl %esi
	pushl %ebp
	pushl %edi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax

	movl %esp, %eax
	andl $0xfffffff0, %esp
	subl $16, %esp
	movl %eax, (%esp)

	call do_intr_time_handler

.global soft_iret
	soft_iret:
		popl %esp

		popl %eax
		popl %ebx
		popl %ecx
		popl %edx
		popl %edi
		popl %ebp
		popl %esi
		popl %esp

		addl $8, %esp
		iret

divide_error:
	pushl $_divide_err
    jmp _no_err_code

debug:
	pushl $do_int3		# _do_debug
	jmp _no_err_code

nmi:
	pushl $do_nmi
	jmp _no_err_code

int3:
	pushl $do_int3
	jmp _no_err_code

overflow:
	pushl $do_overflow
	jmp _no_err_code

bounds:
	pushl $do_bounds
	jmp _no_err_code

invalid_op:
	pushl $do_invalid_op
	jmp _no_err_code

coprocessor_segment_overrun:
	pushl $do_coprocessor_segment_overrun
	jmp _no_err_code

reserved:
	pushl $do_reserved
	jmp _no_err_code   

invalid_TSS:
	pushl $do_invalid_TSS
	jmp _err_code

segment_not_present:
	pushl $do_segment_not_present
	jmp _err_code

stack_segment:
	pushl $do_stack_segment
	jmp _err_code

general_protection:
	pushl $do_general_protection
	jmp _err_code
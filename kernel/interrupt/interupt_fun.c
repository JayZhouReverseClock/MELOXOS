#include <libs/mstdio.h>
#include <kernel/interrupt/interupt_fun.h>
#include <vga/vga.h>
void _divide_err(unsigned long addr, int err_code)
{
    vga_set_show(1,VGA_COLOR_BLACK,VGA_COLOR_BLUE);
    kprintf("divide err addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_double_fault(unsigned long addr, int err_code)
{
	kprintf("double_fault addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_general_protection(unsigned long addr, int err_code)
{
	kprintf("general_protection addr = 0x%x,err code = %d\n", addr,err_code);
}


void do_int3(unsigned long addr, int err_code)
{
    kprintf("int3 addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_nmi(unsigned long addr, int err_code)
{
	kprintf("nmi addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_debug(unsigned long addr, int err_code)
{
	kprintf("debug addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_overflow(unsigned long addr, int err_code)
{
	kprintf("overflow addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_bounds(unsigned long addr, int err_code)
{
	kprintf("bounds addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_invalid_op(unsigned long addr, int err_code)
{
	kprintf("invalid_op addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_device_not_available(unsigned long addr, int err_code)
{
	kprintf("device_not_availabel addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_coprocessor_segment_overrun(unsigned long addr, int err_code)
{
	kprintf("coprprocessor_segment_overrun addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_invalid_TSS(unsigned long addr, int err_code)
{
	kprintf("invalid_TSS addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_segment_not_present(unsigned long addr, int err_code)
{
	kprintf("segment not present addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_stack_segment(unsigned long addr, int err_code)
{
	kprintf("stack_segment addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_coprocessor_error(unsigned long addr, int err_code)
{
    kprintf("coprocessor_err addr = 0x%x,err code = %d\n", addr,err_code);
}

void do_reserved(unsigned long addr, int err_code)
{
	kprintf("reserved addr = 0x%x,err code = %d\n", addr,err_code);
}
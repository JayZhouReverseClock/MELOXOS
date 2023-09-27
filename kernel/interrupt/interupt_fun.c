#include <libs/mstdio.h>
#include <kernel/interrupt/interupt_fun.h>
#include <vga/vga.h>
#include <kernel/process/peocess.h>
#include <kernel/cpu/io.h>

static int_subscriber subscribers[256];

void intr_subscribe(const uint8_t vector, int_subscriber subscriber) {
    subscribers[vector] = subscriber;
}

void intr_unsubscribe(const uint8_t vector, int_subscriber subscriber) {
    if (subscribers[vector] == subscriber) {
        subscribers[vector] = (int_subscriber) 0;
    }
}

void intr_handler(isr_param* param)
{
	__current->intr_contxt = *param;
    
#ifdef USE_KERNEL_PT
    cpu_lcr3(__kernel_ptd);

    vmm_mount_pd(PD_MOUNT_1, __current->page_table);
#endif

    isr_param *lparam = &__current->intr_contxt;
    
    if (lparam->vector <= 255) {
        int_subscriber subscriber = subscribers[lparam->vector];
        if (subscriber) {
            subscriber(param);
            goto done;
        }
    }

    // if (fallback) {
    //     fallback(lparam);
    //     goto done;
    // }
    
    kprintf("INT %x: (%x) [%x: %x] Unknown",
            lparam->vector,
            lparam->err_code,
            lparam->cs,
            lparam->eip);

done:
    // for all external interrupts except the spurious interrupt
    //  this is required by Intel Manual Vol.3A, section 10.8.1 & 10.8.5
    if (lparam->vector >= EXTERN_INTER) {
        pic_end_eoi();
	}

#ifdef USE_KERNEL_PT
    cpu_lcr3(__current->page_table);
#endif
    return;
}

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

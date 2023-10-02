#ifndef __MELOX_IDT__
#define __MELOX_IDT__
#include <stdint.h>
#include <kernel/interrupt/interupt_fun.h>
#define IDT_ATTR(dpl)                   ((0x70 << 5) | (dpl & 3) << 13 | 1 << 15)

extern void intr_syscall_handler(isr_param* param);

void divide_error(void);
void page_exception(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
//void device_not_available(void);
//void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
void page_fault(void);
//void coprocessor_error(void);
void reserved(void);
void parallel_interrupt(void);
//void irq13(void);

void _set_idt_entry(uint32_t vector, uint16_t seg_selector, void (*isr)(), uint8_t dpl);
void _init_idt();
#endif
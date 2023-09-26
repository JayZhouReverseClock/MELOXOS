#include <init/idt.h>
#include <kernel/interrupt/interupt_fun.h>
#define IDT_ENTRY 80

uint64_t _idt[IDT_ENTRY];
uint16_t _idt_limit = sizeof(_idt) - 1;

void _set_idt_entry(uint32_t vector, uint16_t seg_selector, void (*isr)(), uint8_t dpl) {
    uintptr_t offset = (uintptr_t)isr;
    _idt[vector] = (offset & 0xffff0000) | IDT_ATTR(dpl);
    _idt[vector] <<= 32;
    _idt[vector] |= (seg_selector << 16) | (offset & 0x0000ffff);
}

void _init_idt()
{
    _set_idt_entry(FAULT_DIVISION_ERROR, 0x08, &divide_error, 0);
    _set_idt_entry(FAULT_TRAP_DEBUG_EXCEPTION, 0x08, &debug, 0);
    _set_idt_entry(INT_NMI, 0x08, &nmi, 0);
    _set_idt_entry(TRAP_BREAKPOINT, 0x08, &int3, 0);
    _set_idt_entry(TRAP_OVERFLOW, 0x08, &overflow, 0);
    _set_idt_entry(FAULT_BOUND_EXCEED, 0x08, &bounds, 0);
    _set_idt_entry(FAULT_INVALID_OPCODE, 0x08, &invalid_op, 0);
    //_set_idt_entry(FAULT_NO_MATH_PROCESSOR, 0x08, &device_not_available, 0);
    //_set_idt_entry(ABORT_DOUBLE_FAULT, 0x08, &double_fault, 0);
    _set_idt_entry(FAULT_RESERVED_0, 0x08, &coprocessor_segment_overrun, 0);
    _set_idt_entry(FAULT_INVALID_TSS, 0x08, &invalid_TSS, 0);
    _set_idt_entry(FAULT_SEG_NOT_PRESENT, 0x08, &segment_not_present, 0);
    _set_idt_entry(FAULT_STACK_SEG_FAULT, 0x08, &stack_segment, 0);
    _set_idt_entry(FAULT_GENERAL_PROTECTION, 0x08, &general_protection, 0);
    _set_idt_entry(FAULT_PAGE_FAULT, 0x08, &page_fault, 0);
    _set_idt_entry(FAULT_RESERVED_1, 0x08, &reserved, 0);
    //_set_idt_entry(FAULT_X87_FAULT, 0x08, coprocessor_error, 0);
    for (int i=17;i < 48;i++)
		  _set_idt_entry(i, 0x08, &reserved, 0);
    //_set_idt_entry(45, 0x08, irq13, 0);
}
#ifndef __MELOX_IDT__
#define __MELOX_IDT__
#include <stdint.h>

#define IDT_ATTR(dpl)                   ((0x70 << 5) | (dpl & 3) << 13 | 1 << 15)

void _set_idt_entry(uint32_t vector, uint16_t seg_selector, void (*isr)(), uint8_t dpl);
void _init_idt();
#endif
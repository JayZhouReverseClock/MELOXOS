#ifndef __MELOX_IDTHAND__
#define __MELOX_IDTHAND__
#include <kernel/cpu/cpu.h>
#include <kernel/interrupt/int_vector.h>
// Ref: Intel Manuel Vol.3 Figure 6-1


typedef struct {
    gp_regs registers;
    unsigned int vector;
    unsigned int err_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;
} __attribute__((packed)) isr_param;

typedef void (*int_subscriber)(isr_param*);

void _divide_err();

void intr_handler(isr_param* param);

void intr_subscribe(const uint8_t vector, int_subscriber);

void intr_unsubscribe(const uint8_t vector, int_subscriber);


#endif
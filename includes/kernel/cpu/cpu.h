#ifndef CPU_H
#define CPU_H
#include <stdint.h>
typedef unsigned int reg32;
typedef unsigned short reg16;

typedef struct
{
    reg32 eax;
    reg32 ebx;
    reg32 ecx;
    reg32 edx;
    reg32 edi;
    reg32 ebp;
    reg32 esi;
    reg32 esp;
} __attribute__((packed)) gp_regs;

static inline reg32 cpu_rcr2()
{
    uintptr_t val;
    asm volatile("movl %%cr2,%0" : "=r" (val));
    return val;
}

static inline void cpu_lcr3(reg32 v)
{
    asm("mov %0, %%cr3" ::"r"(v));
}

static inline reg32 cpu_rcr3()
{
    uintptr_t val;
    asm volatile("movl %%cr3,%0" : "=r" (val));
    return val;
}

static inline reg32 cpu_reflags()
{
    uintptr_t val;
    asm volatile(
        "pushf\n"
        "popl %0\n"
        :"=r"(val)::);
    return val;
}

reg32 cpu_r_cr0();

reg32 cpu_r_cr2();

reg32 cpu_r_cr3();


static inline void cpu_invplg(void* va)
{
    asm volatile("invlpg (%0)" ::"r"((unsigned int)va) : "memory");
};

void cpu_w_cr0(reg32 v);

void cpu_w_cr2(reg32 v);

void cpu_w_cr3(reg32 v);

void cpu_get_model(char* model_out);

int cpu_brand_string_supported();

void cpu_get_brand(char* brand_out);


#endif
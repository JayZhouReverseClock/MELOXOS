#ifndef CPU_H
#define CPU_H

typedef unsigned long reg32;
typedef unsigned short reg16;


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
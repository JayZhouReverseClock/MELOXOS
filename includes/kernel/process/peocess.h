#ifndef MELOX_PROCESS_H
#define MELOX_PROCESS_H

#include <stdint.h>
#include <kernel/memory/phy_mem.h>
#include <kernel/memory/page.h>
#include <kernel/interrupt/interupt_fun.h>
#include <kernel/memory/mm.h>
#define KERNEL_PID -1

#define PROC_CREATED 0
#define PROC_RUNNING 1
#define PROC_STOPPED 2
#define PROC_TERMNAT 3
#define PROC_DESTROY 4

#define PD_REFERENCED       PT_BASE_VADDR



struct m_pcb{
    pid_t pid;
    struct m_pcb* parent;
    isr_param intr_contxt;
    uint8_t state;
    struct mm_region* process_mm;
    void* page_table;
};


pid_t m_fork();
void init_proc(struct m_pcb* pcb);
void* copy_page(pid_t pid, uintptr_t mount_point);
void* copy_all_page(struct m_pcb* proc, uintptr_t usedMnt);
#endif
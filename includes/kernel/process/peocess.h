#ifndef MELOX_PROCESS_H
#define MELOX_PROCESS_H

#include <stdint.h>
#include <kernel/memory/phy_mem.h>
#include <kernel/memory/page.h>
#include <kernel/interrupt/interupt_fun.h>
#include <kernel/memory/mm.h>
#define KERNEL_PID -1

#define PROC_STOPPED 0
#define PROC_RUNNING 1
#define PROC_TERMNAT 2
#define PROC_DESTROY 4
#define PROC_BLOCKED 8
#define PROC_CREATED 16

#define PROC_TERMMASK 0x6

#define PD_REFERENCED       PT_BASE_VADDR

extern volatile struct m_pcb* __current;//TODO MOVE TO .H AND TODO SCHEDULE

struct m_pcb{
    pid_t pid;
    struct m_pcb* parent;
    isr_param intr_contxt;
    uint8_t state;
    struct llist_header siblings;
    struct llist_header children;
    struct mm_region* process_mm;
    uint32_t pro_ticks;
    int32_t exit_code;
    void* page_table;
    int32_t k_status;
};


pid_t m_fork();
void init_proc(struct m_pcb* pcb);
void* copy_page(pid_t pid, uintptr_t mount_point);
void* copy_all_page(struct m_pcb* proc, uintptr_t usedMnt);
void terminate_proc(int exit_code);
pid_t destroy_process(pid_t pid);

pid_t alloc_pid();
/**
 * @brief 向系统发布一个进程，使其可以被调度。
 * 
 * @param process 
 */
void push_process(struct m_pcb* process);
#endif
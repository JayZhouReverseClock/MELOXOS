#ifndef MELOX_SCHED_H
#define MELOX_SCHED_H
#include <kernel/process/peocess.h>

struct scheduler {
    struct m_pcb* _procs;
    int procs_index;
    unsigned int ptable_len;
};

void sched_init();
void schedule();


#endif
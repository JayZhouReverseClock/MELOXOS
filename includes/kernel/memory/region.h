#ifndef REGION_H
#define REGION_H
#include <kernel/process/peocess.h>
void region_add(struct m_pcb* proc,unsigned long start, unsigned long end, unsigned int attr);
void region_release_all(struct m_pcb* proc);
struct mm_region* region_get(struct m_pcb* proc, unsigned long vaddr);
#endif
#ifndef REGION_H
#define REGION_H
#include <kernel/process/peocess.h>
void region_add(struct m_pcb* proc,unsigned long start, unsigned long end, unsigned int attr);

#endif
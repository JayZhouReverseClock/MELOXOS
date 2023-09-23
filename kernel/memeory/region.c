#include <kernel/memory/region.h>
#include <kernel/memory/malloc.h>
#include <libs/list.h>
void region_add(struct m_pcb* proc,unsigned long start, unsigned long end, unsigned int attr) {
    struct mm_region* region = k_malloc(sizeof(struct mm_region));

    *region = (struct mm_region) {
        .attr = attr,
        .end = end,
        .start = start
    };
    if (!proc->process_mm) {
        llist_init_head(&region->head);
        proc->process_mm = region;
    }
    else {
        llist_append(&proc->process_mm->head, &region->head);
    }
}
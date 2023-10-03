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

void region_release_all(struct m_pcb* proc) {
    struct mm_region* head = proc->process_mm;
    struct mm_region *pos, *n;

    llist_for_each(pos, n, &head->head, head) {
        malloc_free(pos);
    }

    proc->process_mm = NULL;
}

struct mm_region* region_get(struct m_pcb* proc, unsigned long vaddr) {
    struct mm_region* head = proc->process_mm;
    
    if (!head) {
        return NULL;
    }

    struct mm_region *pos, *n;

    llist_for_each(pos, n, &head->head, head) {
        if (vaddr >= pos->start && vaddr < pos->end) {
            return pos;
        }
    }
    return NULL;
}
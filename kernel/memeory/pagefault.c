#include <stdint.h>
#include <kernel/cpu/cpu.h>
#include <kernel/memory/mm.h>
#include <kernel/process/peocess.h>
#include <kernel/memory/region.h>
#include <kernel/memory/vir_mem.h>
#include <kernel/memory/phy_mem.h>
#include <libs/mstdio.h>
#include <status.h>

void do_page_fault(unsigned long err_code, unsigned long address)
{
    uintptr_t ptr = cpu_rcr2();
    if (!ptr) {
        goto segv_term;
    }

    struct mm_region* hit_region = region_get(__current, ptr);
    if (!hit_region) {
        // Into the void...
        goto segv_term;
    }

    ptd_t* pte = PTE_MOUNTED(PD_REFERENCED, ptr >> 12);
    if (*pte & PG_PRESENT) {
        if ((hit_region->attr & REGION_PERM_MASK) ==
            (REGION_RSHARED | REGION_READ)) {
            // normal page fault, do COW
            cpu_invplg(pte);
            uintptr_t pa =
              (uintptr_t)vmm_dup_page(__current->pid, PG_ENTRY_ADDR(*pte));
            pmm_free_page(__current->pid, *pte & ~0xFFF);
            *pte = (*pte & 0xFFF) | pa | PG_WRITE;
            return;
        }
        // impossible cases or accessing privileged page
        goto segv_term;
    }

    if (!(*pte)) {
        // Invalid location
        goto segv_term;
    }
    uintptr_t loc = *pte & ~0xfff;
    // a writable page, not present, pte attr is not null
    //   and no indication of cached page -> a new page need to be alloc
    if ((hit_region->attr & REGION_WRITE) && (*pte & 0xfff) && !loc) {
        cpu_invplg(pte);
        uintptr_t pa = pmm_alloc_page(__current->pid, 0);
        *pte = *pte | pa | PG_PRESENT;
        return;
    }
    // page not present, bring it from disk or somewhere else
    kprintf("WIP page fault route,errcode = 0x%x,wrong address = 0x%x, pageaddress is 0x%x",err_code, address, ptr);
    while (1)
        ;

segv_term:
    kprintf("[KERROR] (pid: %d) Segmentation fault on %x (%x)\n",
            __current->pid,
            ptr,
            address);
    terminate_proc(MXSEGFAULT);
    while(1);
    // should not reach
}
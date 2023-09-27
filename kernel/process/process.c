#include <kernel/process/peocess.h>
#include <kernel/memory/vir_mem.h>
#include <libs/mstring.h>
#include <kernel/memory/region.h>
#include <syscall/syscall.h>

extern volatile struct m_pcb* __current;//TODO MOVE TO .H AND TODO SCHEDULE

__DEFINE_MXSYSCALL(pid_t, fork)
{
    return m_fork();
}

__DEFINE_MXSYSCALL(pid_t, getpid)
{
    return __current->pid;
}

__DEFINE_MXSYSCALL(pid_t, getppid)
{
    return __current->parent->pid;
}

void init_proc(struct m_pcb* pcb)
{
    memset(pcb, 0, sizeof(*pcb));

    pcb->pid = alloc_pid();
    pcb->state = PROC_CREATED;
    pcb->pro_ticks = 10;
}

void* copy_page(pid_t pid, uintptr_t mount_point)
{
    void* pid_ptd = pmm_alloc_page(pid, PP_FGPERSIST);
    ptd_t* vpid_ptd = vmm_cover_map_page(pid, PD_MOUNT_1, pid_ptd, PG_PREM_RW, PG_PREM_RW);
    ptd_t* mount_ptd = (ptd_t*)(mount_point | (0x3FFU << 12));

    for(size_t i = 0; i < PG_MAX_ENTRIES - 1; i++)
    {
        ptd_t mount_ptde = mount_ptd[i];
        if(!mount_point || !(mount_ptde & PG_PRESENT))
        {
            vpid_ptd[i] = mount_ptde;
            continue;
        }

        void* pid_pt = pmm_alloc_page(pid, PP_FGPERSIST);
        pt_t* vpid_pt = vmm_cover_map_page(pid, PG_MOUNT_2, pid_pt, PG_PREM_RW, PG_PREM_RW);
        pt_t* mount_pt = (ptd_t*)(mount_point | (i << 12));

        for(size_t j = 0; j < PG_MAX_ENTRIES; j++)
        {
            pt_t mount_pte = mount_pt[j];
            pmm_ref_page(pid, PG_ENTRY_ADDR(mount_pte));
            vpid_pt[j] = mount_pte;
        }

        vpid_ptd[i] = (uintptr_t)pid_pt | PG_PREM_RW;
    }

    vpid_ptd[PG_MAX_ENTRIES - 1] = PDE(T_SELF_REF_PERM, pid_ptd);

    return pid_ptd;
}

void* copy_all_page(struct m_pcb* proc, uintptr_t usedMnt)
{
    // copy the entire kernel page table
    pid_t pid = proc->pid;
    void* pt_copy = copy_page(pid, usedMnt);

    vmm_mount_pd(PD_MOUNT_2, pt_copy); // 将新进程的页表挂载到挂载点#2

    // copy the kernel stack
    for (size_t i = KSTACK_START >> 12; i <= KSTACK_TOP >> 12; i++) {
        volatile pt_t* ppte = &PTE_MOUNTED(PD_MOUNT_2, i);

        /*
            The TLB caching keep the rewrite to PTE
            from updating. 
        */
        cpu_invplg(ppte);

        pt_t p = *ppte;
        void* ppa = vmm_dup_page(pid, PG_ENTRY_ADDR(p));
        *ppte = (p & 0xfff) | (uintptr_t)ppa;
    }

    // 我们不需要分配内核的区域，因为所有的内核代码和数据段只能通过系统调用来访问，任何非法的访问
    // 都会导致eip落在区域外面，从而segmentation fault.

    // 定义用户栈区域，但是不分配实际的物理页。我们会在Page fault
    // handler里面实现动态分配物理页的逻辑。

    proc->page_table = pt_copy;
}

pid_t m_fork()
{
    struct m_pcb curr_pcb;
    init_proc(&curr_pcb);

    curr_pcb.process_mm = __current->process_mm;
    curr_pcb.intr_contxt = __current->intr_contxt;
    curr_pcb.parent = __current;

#ifdef USE_KERNEL_PG
    copy_all_page(&pcb, PD_MOUNT_1); //挂载点#1是当前进程的页表
#else
    copy_all_page(&curr_pcb, PD_REFERENCED);
#endif

        // 根据 mm_region 进一步配置页表
    if (!__current->process_mm) {
        goto not_copy;
    }

    llist_init_head(&curr_pcb.process_mm);
    struct mm_region *pos, *n;
    llist_for_each(pos, n, &__current->process_mm->head, head)
    {
        region_add(&curr_pcb, pos->start, pos->end, pos->attr);

        // 如果写共享，则不作处理。
        if ((pos->attr & REGION_WSHARED)) {
            continue;
        }

        uintptr_t start_vpn = PG_ALIGN(pos->start) >> 12;
        uintptr_t end_vpn = PG_ALIGN(pos->end) >> 12;
        for (size_t i = start_vpn; i < end_vpn; i++) {
            pt_t* curproc = &PTE_MOUNTED(PD_MOUNT_1, i);
            pt_t* newproc = &PTE_MOUNTED(PD_MOUNT_2, i);
            cpu_invplg(newproc);

            if (pos->attr == REGION_RSHARED) {
                // 如果读共享，则将两者的都标注为只读，那么任何写入都将会应用COW策略。
                cpu_invplg(curproc);
                *curproc = *curproc & ~PG_WRITE;
                *newproc = *newproc & ~PG_WRITE;
            } else {
                // 如果是私有页，则将该页从新进程中移除。
                *newproc = 0;
            }
        }
    }

not_copy:
    vmm_unmount_pd(PD_MOUNT_2);

    // 正如同fork，返回两次。
    curr_pcb.intr_contxt.registers.eax = 0;

    push_process(&curr_pcb);

    return curr_pcb.pid;
}

void __del_pagetable(pid_t pid, uintptr_t mount_point)
{
    ptd_t* pptd = (ptd_t*)(mount_point | (0x3FF << 12));

    for (size_t i = 0; i < PG_MAX_ENTRIES - 1; i++) {
        ptd_t ptde = pptd[i];
        if (!ptde || !(ptde & PG_PRESENT)) {
            continue;
        }

        pt_t* ppt = (pt_t*)(mount_point | (i << 12));

        for (size_t j = 0; j < PG_MAX_ENTRIES; j++) {
            pt_t pte = ppt[j];
            // free the 4KB data page
            if ((pte & PG_PRESENT)) {
                pmm_free_page(pid, PG_ENTRY_ADDR(pte));
            }
        }
        // free the L2 page table
        pmm_free_page(pid, PG_ENTRY_ADDR(ptde));
    }
    // free the L1 directory
    pmm_free_page(pid, PG_ENTRY_ADDR(pptd[PG_MAX_ENTRIES - 1]));
}
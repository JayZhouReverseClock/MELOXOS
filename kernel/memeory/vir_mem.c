#include <kernel/memory/vir_mem.h>
#include <libs/mstring.h>

void vmm_init() {
    // TODO: something here?
}

// TODO: Move these nasty inline asm stuff into hal
//      These should be arch dependent
ptd_t* get_pd() {
    ptd_t* pd;
    #ifdef __ARCH_IA32
    __asm__(
        "movl %%cr3, %0\n"
        "andl $0xfffff000, %0"
        : "=r"(pd)
    );
    #endif
    return P2V(pd);
}

void set_pd(ptd_t* pd) {
    #ifdef __ARCH_IA32
    __asm__(
        "movl %0, %%eax\n"
        "andl $0xfffff000, %%eax\n"
        "movl %%eax, %%cr3\n"
        :
        : "r" (pd)
    );
    #endif
}

ptd_t* vmm_init_pd() {
    ptd_t* dir = pmm_alloc_page(KERNEL_PID, PP_FGPERSIST);
    for (size_t i = 0; i < 1024; i++)
    {
        dir[i] = 0;
    }
    
    // 自己映射自己，方便我们在软件层面进行查表地址转换
    dir[1023] = PDE(T_SELF_REF_PERM, dir);

    return dir;
}
#ifdef BITMAP
void* vmm_map_page(void* va, void* pa, pt_attr dattr, pt_attr tattr) {
    // 显然，对空指针进行映射没有意义。
    if (!pa || !va) {
        return NULL;
    }

    uintptr_t pd_offset = PD_INDEX(va);
    uintptr_t pt_offset = PT_INDEX(va);
    ptd_t* ptd = (ptd_t*)PTD_BASE_VADDR;

    // 在页表与页目录中找到一个可用的空位进行映射（位于va或其附近）
    ptd_t* pde = ptd[pd_offset];
    pt_t* pt = (uintptr_t)PT_VADDR(pd_offset);
    while (pde && pd_offset < 1024) {
        if (pt_offset == 1024) {
            pd_offset++;
            pt_offset = 0;
            pde = ptd[pd_offset];
            pt = (pt_t*)PT_VADDR(pd_offset);
        }
        // 页表有空位，只需要开辟一个新的 PTE
        if (pt && !pt[pt_offset]) {
            pt[pt_offset] = PTE(tattr, pa);
            return V_ADDR(pd_offset, pt_offset, PG_OFFSET(va));
        }
        pt_offset++;
    }
    
    // 页目录与所有页表已满！
    if (pd_offset > 1024) {
        return NULL;
    }

    // 页目录有空位，需要开辟一个新的 PDE
    uint8_t* new_pt_pa = pmm_alloc_page();
    
    // 物理内存已满！
    if (!new_pt_pa) {
        return NULL;
    }
    
    ptd[pd_offset] = PDE(dattr, new_pt_pa);
    
    memset((void*)PT_VADDR(pd_offset), 0, PM_PAGE_SIZE);
    pt[pt_offset] = PTE(tattr, pa);

    return V_ADDR(pd_offset, pt_offset, PG_OFFSET(va));
}

void* vmm_alloc_page(void* vpn, pt_attr dattr, pt_attr tattr) {
    void* pp = pmm_alloc_page();
    void* result = vmm_map_page(vpn, pp, dattr, tattr);
    if (!result) {
        pmm_free_page(pp);
    }
    return result;
}

void vmm_unmap_page(void* vpn) {
    uintptr_t pd_offset = PD_INDEX(vpn);
    uintptr_t pt_offset = PT_INDEX(vpn);
    ptd_t* self_pde = PTD_BASE_VADDR;

    ptd_t pde = self_pde[pd_offset];

    if (pde) {
        pt_t* pt = (pt_t*)PT_VADDR(pd_offset);
        uint32_t pte = pt[pt_offset];
        if (IS_CACHED(pte) && pmm_free_page(pte)) {
            // 刷新TLB
            #ifdef __ARCH_IA32
            __asm__("invlpg (%0)" :: "r"((uintptr_t)vpn) : "memory");
            #endif
        }
        //if(pt_offset != 8)
        pt[pt_offset] = 0;
    }
}

void* vmm_v2p(void* va) {
    uintptr_t pd_offset = PD_INDEX(va);
    uintptr_t pt_offset = PT_INDEX(va);
    uintptr_t po = PG_OFFSET(va);
    ptd_t* self_pde = PTD_BASE_VADDR;

    ptd_t pde = self_pde[pd_offset];
    if (pde) {
        pt_t pte = ((pt_t*)PT_VADDR(pd_offset))[pt_offset];
        if (pte) {
            uintptr_t ppn = pte >> 12;
            return (void*)P_ADDR(ppn, po);
        }
    }

    return NULL;
}
#endif

#ifdef MEMSTRUCT
int __vmm_map_alloc(pid_t pid, 
                   uint32_t l1_inx,
                   uint32_t l2_inx,
                   uintptr_t pa,
                   pt_attr attr,
                   int forced)
{
    ptd_t* ptd = (ptd_t*)PTD_BASE_VADDR;
    pt_t* pt = (uintptr_t)PT_VADDR(l1_inx);


    // See if attr make sense
    assert(attr <= 128);

    if (!ptd[l1_inx]) {
        uint8_t* new_pt_pa = pmm_alloc_page(pid, PP_FGPERSIST);

        // 物理内存已满！
        if (!new_pt_pa) {
            return 0;
        }

        // This must be writable
        ptd[l1_inx] = PDE(attr | PG_WRITE, new_pt_pa);
        memset((void*)PT_VADDR(l1_inx), 0, PM_PAGE_SIZE);
    }

    if (pt[l2_inx]) {
        if (!forced) {
            return 0;
        }
    }

    if ((HAS_FLAGS(attr, PG_PRESENT))) {
        // add one on reference count, regardless of existence.
        pmm_ref_page(pid, pa);
    }

    pt[l2_inx] = PTE(attr, pa);

    return 1;
}

void* vmm_map_page(pid_t pid, void* va, void* pa, pt_attr dattr, pt_attr tattr) {
    // 显然，对空指针进行映射没有意义。
    if (!pa || !va) {
        return NULL;
    }

    uintptr_t pd_offset = PD_INDEX(va);
    uintptr_t pt_offset = PT_INDEX(va);
    ptd_t* ptd = (ptd_t*)PTD_BASE_VADDR;

    // 在页表与页目录中找到一个可用的空位进行映射（位于va或其附近）
    ptd_t* pde = ptd[pd_offset];
    pt_t* pt = (uintptr_t)PT_VADDR(pd_offset);
    while (pde && pd_offset < 1024) {
        if (pt_offset == 1024) {
            pd_offset++;
            pt_offset = 0;
            pde = ptd[pd_offset];
            pt = (pt_t*)PT_VADDR(pd_offset);
        }
        // 页表有空位，只需要开辟一个新的 PTE
        if (pt && !pt[pt_offset]) {
            pt[pt_offset] = PTE(tattr, pa);
            return V_ADDR(pd_offset, pt_offset, PG_OFFSET(va));
        }
        pt_offset++;
    }
    
    // 页目录与所有页表已满！
    if (pd_offset > 1024) {
        return NULL;
    }

    // 页目录有空位，需要开辟一个新的 PDE
    uint8_t* new_pt_pa = pmm_alloc_page(pid, PP_FGPERSIST);
    
    // 物理内存已满！
    if (!new_pt_pa) {
        return NULL;
    }
    
    ptd[pd_offset] = PDE(dattr, new_pt_pa);
    
    memset((void*)PT_VADDR(pd_offset), 0, PM_PAGE_SIZE);


    if ((HAS_FLAGS(tattr, PG_PRESENT) && HAS_FLAGS(dattr, PG_PRESENT))) {
        // add one on reference count, regardless of existence.
        pmm_ref_page(pid, pa);
    }
    pt[pt_offset] = PTE(tattr, pa);

    return V_ADDR(pd_offset, pt_offset, PG_OFFSET(va));
}

void* vmm_cover_map_page(pid_t pid, void* va, void* pa, pt_attr dattr, pt_attr tattr)
{
        // 显然，对空指针进行映射没有意义。
    if (!pa || !va) {
        return NULL;
    }

    uintptr_t pd_offset = PD_INDEX(va);
    uintptr_t pt_offset = PT_INDEX(va);
    ptd_t* ptd = (ptd_t*)PTD_BASE_VADDR;

    // 在页表与页目录中找到一个可用的空位进行映射（位于va或其附近）
    ptd_t* pde = ptd[pd_offset];
    pt_t* pt = (uintptr_t)PT_VADDR(pd_offset); 
    if(!pde)
    {
        uint8_t* new_pt_pa = pmm_alloc_page(pid, PP_FGPERSIST);
        if (!new_pt_pa) {
            return NULL;
        }

        ptd[pd_offset] = PDE(dattr, new_pt_pa);
        memset((void*)PT_VADDR(pd_offset), 0, PM_PAGE_SIZE);

    }   

    // 页目录有空位，需要开辟一个新的 PDE
    uint8_t* new_pt_pa = pmm_alloc_page(pid, PP_FGPERSIST);
    
    // 物理内存已满！
    if (!new_pt_pa) {
        return NULL;
    }
    
    ptd[pd_offset] = PDE(dattr, new_pt_pa);
    
    memset((void*)PT_VADDR(pd_offset), 0, PM_PAGE_SIZE);

    if (HAS_FLAGS(tattr, PG_PRESENT) && HAS_FLAGS(dattr, PG_PRESENT)) {
        // add one on reference count, regardless of existence.
        pmm_ref_page(pid, pa);
    }
    pt[pt_offset] = PTE(tattr, pa);

    cpu_invplg(va);

    return V_ADDR(pd_offset, pt_offset, PG_OFFSET(va));
}

void* vmm_alloc_page(pid_t pid, void* vpn, void** pa, pt_attr dattr, pt_attr tattr)
{
    void* pp = pmm_alloc_page(pid, tattr);
    void* result = vmm_map_page(pid, vpn, pp, dattr, tattr);
    if (!result) {
        pmm_free_page(pid, pp);
    }
    pa ? (*pa = pp) : 0;
    return result;
}

int vmm_alloc_pages(pid_t pid, void* va, size_t sz, pt_attr tattr, pp_attr_t pattr)
{
    assert((uintptr_t)va % PG_SIZE == 0) assert(sz % PG_SIZE == 0);

    void* va_ = va;
    for (size_t i = 0; i < (sz >> PG_SIZE_BITS); i++, va_ += PG_SIZE) {
        void* pp = pmm_alloc_page(pid, pattr);
        uint32_t l1_index = PD_INDEX(va_);
        uint32_t l2_index = PT_INDEX(va_);
        if (!pp || !__vmm_map_alloc(
                     pid,
                     l1_index, l2_index, (uintptr_t)pp, tattr, false)) {
            // if one failed, release previous allocated pages.
            va_ = va;
            for (size_t j = 0; j < i; j++, va_ += PG_SIZE) {
                vmm_unmap_page(pid, va_);
            }

            return false;
        }
    }

    return true;
}

int vmm_set_mapping(pid_t pid, void* va, void* pa, pt_attr attr)
{
    assert(((uintptr_t)va & 0xFFFU) == 0);

    uint32_t l1_index = PD_INDEX(va);
    uint32_t l2_index = PT_INDEX(va);

    // prevent map of recursive mapping region
    if (l1_index == 1023) {
        return 0;
    }
    
    __vmm_map_alloc(pid, l1_index, l2_index, (uintptr_t)pa, attr, false);
    return 1;
}

void __vmm_unmap_unalloc(pid_t pid, void* va, int free_ppage)
{
    assert(((uintptr_t)va & 0xFFFU) == 0);

    uint32_t l1_index = PD_INDEX(va);
    uint32_t l2_index = PT_INDEX(va);

    // prevent unmap of recursive mapping region
    if (l1_index == 1023) {
        return;
    }

    ptd_t* ptd = (ptd_t*)PTD_BASE_VADDR;

    if (ptd[l1_index]) {
        pt_t* pt = (uintptr_t)PT_VADDR(l1_index);
        pt_t pte = pt[l2_index];
        if (IS_CACHED(pte) && free_ppage) {
            pmm_free_page(pid, (void*)pte);
        }
        cpu_invplg(va);

        pt[l2_index] = 0;
    }
}

void vmm_unmap_page(pid_t pid, void* va)
{
    __vmm_unmap_unalloc(pid, va, true);
}

void vmm_unset_mapping(void* va)
{
    __vmm_unmap_unalloc(0, va, false);
}

v_mapping vmm_lookup(void* va)
{
    assert(((uintptr_t)va & 0xFFFU) == 0);

    uint32_t l1_index = PD_INDEX(va);
    uint32_t l2_index = PT_INDEX(va);

    ptd_t* ptd = (ptd_t*)PTD_BASE_VADDR;

    v_mapping mapping = { .flags = 0, .pa = 0, .pn = 0 };
    if (ptd[l1_index]) {
        pt_t* pt = (uintptr_t)PT_VADDR(l1_index);
        pt_t* l2pte = &pt[l2_index];
        if (l2pte) {
            mapping.flags = PG_ENTRY_FLAGS(*l2pte);
            mapping.pa = PG_ENTRY_ADDR(*l2pte);
            mapping.pn = mapping.pa >> PG_SIZE_BITS;
            mapping.pte = l2pte;
        }
    }

    return mapping;
}

void* vmm_v2p(void* va)
{
    return (void*)vmm_lookup(va).pa;
}
#endif
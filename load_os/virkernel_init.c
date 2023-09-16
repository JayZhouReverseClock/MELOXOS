#include <init/multiboot.h>
#include <stdint.h>
#include <kernel/memory/page.h>
#include <libs/mstdio.h>
#include <libs/mstdlib.h>

#define PT_ADDR(ptd, pt_index)                       ((ptd_t*)ptd + (pt_index + 1) * 1024)
#define sym_val(sym)                                 (uintptr_t)(&sym)
#define MEM_1M 0X100000
#define KERNEL_PAGE_COUNT           ((sym_val(virkernel_end) - sym_val(virkernel_start) + 4096 - 1) >> 12)
#define LOADOS_PAGE_COUNT              ((sym_val(__init_phykernel_end) - MEM_1M + 4096 - 1) >> 12)

// use table #1
#define PG_TABLE_IDENTITY           0

// use table #2-4
// hence the max size of kernel is 8MiB
#define PG_TABLE_KERNEL             1

// use table #5
#define PG_TABLE_STACK              4
// Provided by linker (see linker.ld)
extern uint8_t virkernel_start;
extern uint8_t virkernel_end ;
extern uint8_t __init_phykernel_end;
extern uint8_t _kernel_stack;

uint32_t _save_(uint8_t* destination, uint8_t* base, multiboot_uint32_t size) {
    multiboot_uint32_t i = 0;
    for (; i < size; i++)
    {
        *(destination + i) = *(base + i);
    }
    return i;
}

void save_multiboot_info(multiboot_info_t* mb_info, multiboot_uint8_t* dest)
{
    uint32_t current = 0;
    uint8_t* info_b = (uint8_t*) mb_info;
    for (; current < sizeof(multiboot_info_t); current++)
    {
        *(dest + current) = *(info_b + current);
    }

    ((multiboot_info_t*) dest)->mmap_addr = (uintptr_t)dest + current;
    current += _save_(dest + current, (uint8_t*)mb_info->mmap_addr, mb_info->mmap_length);

    if (present(mb_info->flags, MULTIBOOT_INFO_DRIVE_INFO)) {
        ((multiboot_info_t*) dest)->drives_addr = (uintptr_t)dest + current;
        current += _save_(dest + current, (uint8_t*)mb_info->drives_addr, mb_info->drives_length);
    }
    multiboot_info_t* test = (multiboot_info_t*) dest;
    multiboot_memory_map_t* testmap = (multiboot_memory_map_t*)mb_info->mmap_addr;
}

void init_page(ptd_t* pt)
{
    SET_PDE(pt, 0, PDE(PG_PRESENT, pt + PG_MAX_ENTRIES));
    ptd_t * tmp_paged_phy_addr = pt;
    ptd_t * tmp_paget_phy_addr = pt + 1024;
    //identity map 1M memory
    for(uint32_t i = 0; i < 256; i++)
        *(tmp_paget_phy_addr + i) = (((i << 12) & 0xFFFFF000UL) | (PG_PREM_RW & 0xfff));

    //identity map virkernel_init memory
    for(uint32_t i = 0; i < LOADOS_PAGE_COUNT; i++)
        *(tmp_paget_phy_addr + i + 256) = ((((i << 12) + MEM_1M) & 0xFFFFF000UL) | (PG_PREM_RW & 0xfff));

    //now we map the kernel
    //calculate the virtrue kernrl page index
    uint32_t virk_pde_index = PD_INDEX(sym_val(virkernel_start));
    uint32_t virk_pte_index = PT_INDEX(sym_val(virkernel_start));
    uint32_t virk_pg_counts = KERNEL_PAGE_COUNT;

    //regist the vir_kernel page to page direction
    //we give 3 pagetable to virkel, is 3 * 4 = 12 mib size
    for(uint32_t i = 0; i < PG_TABLE_STACK - PG_TABLE_KERNEL; i++)
    {
        *(pt + virk_pde_index + i) = PDE(PG_PREM_RW, PT_ADDR(pt, PG_TABLE_KERNEL + i));
    }

    //avoid kernel size > prepared page count
    if(virk_pg_counts > (PG_TABLE_STACK - PG_TABLE_KERNEL) * 1024)
    {
        while(1);
    }

    //calcul the phy address
    uint32_t virk_phy_addr = V2P(&virkernel_start);
    //map virtrue kernel page
    for(uint32_t i = 0; i < virk_pg_counts; i++)
    {
        *(tmp_paget_phy_addr + 1024 + virk_pte_index + i) = (((virk_phy_addr + (i << 12)) & 0xFFFFF000UL) | (PG_PREM_RW & 0xfff));
    }

    //self map pd, to give the pd virture address
    SET_PDE(pt, 1023, PDE(T_SELF_REF_PERM, pt));
}

void kerner_init(ptd_t* pt, uint32_t pt_size)
{
    uint8_t* kpg = (uint8_t*) pt;
    for (uint32_t i = 0; i < pt_size; i++)
    {
        *(kpg + i) = 0;
    }
    init_page(pt);
}
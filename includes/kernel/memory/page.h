#ifndef MELOX_PAGE_H
#define MELOX_PAGE_H

#include <stdint.h>
typedef unsigned int ptd_t;
typedef unsigned int pt_t;
typedef unsigned int pt_attr;

typedef struct {
    // 物理页码（如果不存在映射，则为0）
    uint32_t pn;
    // 物理页地址（如果不存在映射，则为0）
    uintptr_t pa;
    // 映射的flags
    uint16_t flags;
    // PTE地址
    uint32_t *pte;
} v_mapping;

#define K_STACK_SIZE            0x100000U
#define K_STACK_START           ((0xFFBFFFFFU - K_STACK_SIZE) + 1)
// 页目录的虚拟基地址，可以用来访问到各个PDE
#define PTD_BASE_VADDR                0xFFFFF000U

// 页表的虚拟基地址，可以用来访问到各个PTE
#define PT_BASE_VADDR                 0xFFC00000U

#define PG_SIZE_BITS                12
#define PG_SIZE                     (1 << PG_SIZE_BITS)

#define HAS_FLAGS(entry, flags)             ((PG_ENTRY_FLAGS(entry) & (flags)) == flags)

// 用来获取特定的页表的虚拟地址
#define PT_VADDR(pd_offset)           (PT_BASE_VADDR | (pd_offset << 12))

#define V_ADDR(pd, pt, offset)  ((pd) << 22 | (pt) << 12 | (offset))
#define P_ADDR(ppn, offset)     ((ppn << 12) | (offset))


#define PG_ENTRY_FLAGS(entry)   (entry & 0xFFFU)
#define PG_ENTRY_ADDR(entry)   (entry & ~0xFFFU)

#define MEM_1MB                 0x100000UL
#define HIGHER_HLF_BASE         0xC0000000UL

#define PG_MAX_ENTRIES              1024U
#define PG_SIZE_BITS                12
#define PG_PRESENT              (0x1)
#define PG_WRITE                (0x1 << 1)
#define PG_ALLOW_USER           (0x1 << 2)
#define PG_WRITE_THROUGH      (1 << 3)
#define PG_DISABLE_CACHE        (1 << 4)
#define PG_DIRTY(pte)           ((pte & (1 << 6)) >> 6)
#define PG_ACCESSED(pte)        ((pte & (1 << 5)) >> 5)
#define PG_PDE_4MB              (1 << 7)


#define PG_PREM_R              PG_PRESENT
#define PG_PREM_RW             PG_PRESENT | PG_WRITE
#define PG_PREM_UR             PG_PRESENT | PG_ALLOW_USER
#define PG_PREM_URW            PG_PRESENT | PG_WRITE | PG_ALLOW_USER
#define T_SELF_REF_PERM        PG_PREM_RW | PG_DISABLE_CACHE | PG_WRITE_THROUGH

#define IS_CACHED(entry)    ((entry & 0x1))
#define PG_ALIGN(addr)          ((uintptr_t)(addr)  & 0xFFFFF000UL)
#define PDE(flags, pt_addr)     (PG_ALIGN(pt_addr) | (((flags) | PG_WRITE_THROUGH) & 0xfff))
#define PTE(flags, pg_addr)     (PG_ALIGN(pg_addr) | ((flags) & 0xfff))

#define PD_INDEX(vaddr)     (((uintptr_t)(vaddr) & 0xFFC00000UL) >> 22)
#define PT_INDEX(vaddr)     (((uintptr_t)(vaddr) & 0x003FF000UL) >> 12)
#define PG_OFFSET(vaddr)    ((uintptr_t)(vaddr)  & 0x00000FFFUL)

#define PT_ADDR(ptd, pt_index)                       ((ptd_t*)ptd + (pt_index + 1) * 1024)
#define SET_PDE(ptd, pde_index, pde)                 *((ptd_t*)ptd + pde_index) = pde
#define SET_PTE(ptd, pt_index, pte_index, pte)       *(PT_ADDR(ptd, pt_index) + pte_index) = pte

#define P2V(paddr)          ((uintptr_t)(paddr)  +  HIGHER_HLF_BASE)
#define V2P(vaddr)          ((uintptr_t)(vaddr)  -  HIGHER_HLF_BASE)

#define PD_MOUNT_1          0xAFC00000
#define PD_MOUNT_2          0xAF800000
#define PG_MOUNT_BASE       0xAF7FF000
#define PG_MOUNT_1          (PG_MOUNT_BASE)
#define PG_MOUNT_2          (PG_MOUNT_BASE - 0x1000)
#define PG_MOUNT_3          (PG_MOUNT_BASE - 0x2000)
#define PG_MOUNT_4          (PG_MOUNT_BASE - 0x3000)

#define PTE_MOUNTED(mnt, vpn)     (((pt_t*)((mnt) | (((vpn) & 0xffc00) << 2)))[(vpn) & 0x3ff])

#endif
#ifndef PHYMEM_H
#define PHYMEM_H
#include <stdint.h>
#include <stddef.h>
#define MEMSTRUCT

#define PM_PAGE_SIZE            4096
#define PM_BMP_MAX_SIZE (128 * 1024)
#define PM_STRUCT_MAX_SIZE (1024 * 1024)
#define LOOKUP_START 1
#define PP_FGPERSIST            0x1

typedef int32_t pid_t;
typedef uint32_t pp_attr_t;

//we need to refactor the phy membit_map, cause the process need to share the same phy page
//and the membit_map can not support share, we have 2 ways to refactor
//1. change the bitmap to butemap, then cause we do not use bit wo present phyaddr, we use byte, we can count the shared num
//2. change the bitmap to a struct, the struct can have more information
//we decide to use struct
struct pp_struct {
    pid_t owner; //phy process oner
    uint32_t ref_counts; //phy addr reference count
    pp_attr_t attr; //phy attribiute, noW is useless
};

#ifdef BITMAP
//follows is the bitmapuse
/**
 * @brief 标注物理页为可使用
 * 
 * @param ppn page number
 */
void pmm_mark_page_free(uintptr_t ppn);

/**
 * @brief 标注物理页为已占用
 * 
 * @param ppn 
 */
void pmm_mark_page_occupied(uintptr_t ppn);

/**
 * @brief 标注多个连续的物理页为可用
 * 
 * @param start_ppn 起始PPN
 * @param page_count 数量
 */
void pmm_mark_chunk_free(uintptr_t start_ppn, size_t page_count);

/**
 * @brief 标注多个连续的物理页为已占用
 * 
 * @param start_ppn 起始PPN
 * @param page_count 数量
 */
void pmm_mark_chunk_occupied(uintptr_t start_ppn, size_t page_count);

/**
 * @brief 初始化物理内存管理器
 * 
 * @param mem_upper_lim 最大可用内存地址
 */
void bitmap_pmm_init(uintptr_t mem_upper_lim);

/**
 * @brief 分配一个可用的物理页
 * 
 * @return void* 可用的页地址，否则为 NULL
 */
void* bitmap_pmm_alloc_page();

/**
 * @brief 释放一个已分配的物理页，假若页地址不存在，则无操作。
 * 
 * @param page 页地址
 * @return 是否成功
 */
int bitmap_pmm_free_page(void* page);
#endif

//follows is the struct use
#ifdef MEMSTRUCT
/**
 * @brief 标注物理页为可使用
 * 
 * @param ppn page number
 */
void pmm_mark_page_free(uintptr_t ppn);

/**
 * @brief 标注物理页为已占用
 * 
 * @param ppn 
 */
void pmm_mark_page_occupied(pid_t owner, uintptr_t ppn, pp_attr_t attr);

/**
 * @brief 标注多个连续的物理页为可用
 * 
 * @param start_ppn 起始PPN
 * @param page_count 数量
 */
void pmm_mark_chunk_free(uintptr_t start_ppn, size_t page_count);

/**
 * @brief 标注多个连续的物理页为已占用
 * 
 * @param start_ppn 起始PPN
 * @param page_count 数量
 */
void pmm_mark_chunk_occupied(pid_t owner, uintptr_t start_ppn, size_t page_count, pp_attr_t attr);


/**
 * @brief 分配一个可用的物理页
 * 
 * @return void* 可用的页地址，否则为 NULL
 */
void* pmm_alloc_page(pid_t owner, pp_attr_t attr);

/**
 * @brief 分配一个连续的物理内存区域
 * 
 * @param owner 
 * @param num_pages 区域大小，单位为页
 * @param attr 
 * @return void* 
 */
void* pmm_alloc_cpage(pid_t owner, size_t num_pages, pp_attr_t attr);

/**
 * @brief 初始化物理内存管理器
 * 
 * @param mem_upper_lim 最大可用内存地址
 */
void pmm_init(uintptr_t mem_upper_lim);


/**
 * @brief 释放一个已分配的物理页，假若页地址不存在，则无操作。
 * 
 * @param page 页地址
 * @return 是否成功
 */
int pmm_free_page(pid_t owner, void* page);

struct pp_struct* pmm_query(void* pa);

int pmm_ref_page(pid_t owner, void* page);
#endif

#endif
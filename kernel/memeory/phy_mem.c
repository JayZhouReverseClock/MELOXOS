#include <kernel/memory/phy_mem.h>
#include <kernel/memory/page.h>
size_t pg_lookup_ptr;
uintptr_t max_pg;

#ifdef BITMAP
uint8_t pm_bitmap[PM_BMP_MAX_SIZE];
void pmm_mark_page_free(uintptr_t ppn)
{
    pm_bitmap[ppn / 8] = pm_bitmap[ppn / 8] & (0xff7fU >> (ppn % 8));
}

void pmm_mark_page_occupied(uintptr_t ppn)
{
    pm_bitmap[ppn / 8] = pm_bitmap[ppn / 8] | (0x80U >> (ppn % 8));
}

void pmm_mark_chunk_free(uintptr_t start_ppn, size_t page_count)
{
    for(int i = start_ppn; i < page_count; i++)
        pm_bitmap[i / 8] = pm_bitmap[i / 8] & (0xff7fU >> (i % 8));
}

void pmm_mark_chunk_occupied(uintptr_t start_ppn, size_t page_count)
{
    for(int i = start_ppn; i < page_count; i++)
        pm_bitmap[i / 8] = pm_bitmap[i / 8] | (0x80U >> (i % 8));
}

void bitmap_pmm_init(uintptr_t mem_upper_lim)
{
    pg_lookup_ptr = LOOKUP_START;
    max_pg = (PG_ALIGN(mem_upper_lim) >> 12);

    for(int i = 0; i < PM_BMP_MAX_SIZE; i++)
    {
        pm_bitmap[i] = 0xffU;
    }
}

void* bitmap_pmm_alloc_page()
{
    // Next fit approach. Maximize the throughput!
    uintptr_t good_page_found = NULL;
    size_t old_pg_ptr = pg_lookup_ptr;
    size_t upper_lim = max_pg;
    uint8_t chunk = 0;
    while (!good_page_found && pg_lookup_ptr < upper_lim) {
        chunk = pm_bitmap[pg_lookup_ptr >> 3];

        // skip the fully occupied chunk, reduce # of iterations
        if (chunk != 0xFFU) {
            for (size_t i = pg_lookup_ptr % 8; i < 8; i++, pg_lookup_ptr++) {
                if (!(chunk & (0x80U >> i))) {
                    pmm_mark_page_occupied(pg_lookup_ptr);
                    good_page_found = pg_lookup_ptr << 12;
                    break;
                }
            }
        } else {
            pg_lookup_ptr += 8;

            // We've searched the interval [old_pg_ptr, max_pg) but failed
            //   may be chances in [1, old_pg_ptr) ?
            // Let's find out!
            if (pg_lookup_ptr >= upper_lim && old_pg_ptr != LOOKUP_START) {
                upper_lim = old_pg_ptr;
                pg_lookup_ptr = LOOKUP_START;
                old_pg_ptr = LOOKUP_START;
            }
        }
    }
    return (void*)good_page_found;
}

int bitmap_pmm_free_page(void* page)
{
    // TODO: Add kernel reserved memory page check
    uint32_t pg = (uintptr_t)page >> 12;
    if (pg && pg < max_pg)
    {
        pmm_mark_page_free(pg);
        return 1;
    }
    return 0;
}
#endif

#ifdef MEMSTRUCT
static struct pp_struct pm_table[PM_BMP_MAX_SIZE];

void pmm_init(uintptr_t mem_upper_lim)
{
    pg_lookup_ptr = LOOKUP_START;
    max_pg = (PG_ALIGN(mem_upper_lim) >> 12);

    for (size_t i = 0; i < PM_BMP_MAX_SIZE; i++) {
        pm_table[i] = (struct pp_struct) {
            .owner = 0,
            .attr = 0,
            .ref_counts = 1
        };
    }
}

void pmm_mark_page_free(uintptr_t ppn)
{
    pm_table[ppn].ref_counts = 0;
}

void pmm_mark_page_occupied(pid_t owner, uintptr_t ppn, pp_attr_t attr)
{
    pm_table[ppn] = (struct pp_struct) {
        .owner = owner,
        .ref_counts = 1,
        .attr = attr
    };
}

void pmm_mark_chunk_free(uintptr_t start_ppn, size_t page_count)
{
    for (size_t i = start_ppn; i < start_ppn + page_count && i < max_pg; i++)
    {
        pm_table[i].ref_counts = 0;
    }
}

void pmm_mark_chunk_occupied(pid_t owner, uint32_t start_ppn, size_t page_count, pp_attr_t attr)
{
    for (size_t i = start_ppn; i < start_ppn + page_count && i < max_pg; i++)
    {
        pm_table[i] = (struct pp_struct) {
            .owner = owner,
            .ref_counts = 1,
            .attr = attr
        };
    }
}

void* pmm_alloc_page(pid_t owner, pp_attr_t attr)
{
        // Next fit approach. Maximize the throughput!
    uintptr_t good_page_found = NULL;
    size_t old_pg_ptr = pg_lookup_ptr;
    size_t upper_lim = max_pg;
    struct pp_struct* pm;
    while (!good_page_found && pg_lookup_ptr < upper_lim) {
        pm = &pm_table[pg_lookup_ptr];

        // skip the fully occupied chunk, reduce # of iterations
        if (!pm->ref_counts) {
                *pm = (struct pp_struct) {
                .attr = attr,
                .owner = owner,
                .ref_counts = 1
            };
            good_page_found = pg_lookup_ptr << 12;
        } else {
            pg_lookup_ptr ++;

            // We've searched the interval [old_pg_ptr, max_pg) but failed
            //   may be chances in [1, old_pg_ptr) ?
            // Let's find out!
            if (pg_lookup_ptr >= upper_lim && old_pg_ptr != LOOKUP_START) {
                upper_lim = old_pg_ptr;
                pg_lookup_ptr = LOOKUP_START;
                old_pg_ptr = LOOKUP_START;
            }
        }
    }
    // if (!good_page_found) {
    //     __current->k_status = LXOUTOFMEM;
    // }
    return (void*)good_page_found;
}

void* pmm_alloc_cpage(pid_t owner, size_t num_pages, pp_attr_t attr) {
    size_t p1 = 0;
    size_t p2 = 0;

    while (p2 < max_pg && p2 - p1 < num_pages)
    {
        (!(&pm_table[p2])->ref_counts) ? (p2++) : (p1 = p2);
    }

    if (p2 < max_pg) {
        return NULL;
    }

    pmm_mark_chunk_occupied(owner, p1, num_pages, attr);

    return p1 << 12;
}

int pmm_free_page(pid_t owner, void* page)
{
    struct pp_struct* pm = &pm_table[(intptr_t)page >> 12];
    
    // Oops, double free!
    if (!(pm->ref_counts)) {
        return 0;
    }

    // 检查权限，保证：1) 用户只能释放用户页； 2) 内核可释放所有页。
    if ((pm->owner & owner) == pm->owner) {
        pm->ref_counts--;
        return 1;
    }
    return 0;
    
}

struct pp_struct* pmm_query(void* pa) {
    uint32_t ppn = (uintptr_t)pa >> 12;
    
    if (ppn >= PM_BMP_MAX_SIZE) {
        return NULL;
    }

    return &pm_table[ppn];
}

int pmm_ref_page(pid_t owner, void* page) {
    (void*) owner;      // TODO: do smth with owner
    
    uint32_t ppn = (uintptr_t)page >> 12;
    
    if (ppn >= PM_BMP_MAX_SIZE) {
        return 0;
    }

    struct pp_struct* pm = &pm_table[ppn];
    if (!pm->ref_counts) {
        return 0;
    }

    pm->ref_counts++;
    return 1;
}
#endif
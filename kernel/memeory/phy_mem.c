#include <kernel/memory/phy_mem.h>
#include <kernel/memory/page.h>
size_t pg_lookup_ptr;
uint8_t pm_bitmap[PM_BMP_MAX_SIZE];
uintptr_t max_pg;
void pmm_init(uintptr_t mem_upper_lim)
{
    pg_lookup_ptr = LOOKUP_START;
    max_pg = (PG_ALIGN(mem_upper_lim) >> 12);

    for(int i = 0; i < PM_BMP_MAX_SIZE; i++)
    {
        pm_bitmap[i] = 0xffU;
    }
}

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

void* pmm_alloc_page()
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

int
pmm_free_page(void* page)
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
#include <stddef.h>
#include <stdint.h>
#include <kernel/memory/page.h>
#include <kernel/memory/vir_mem.h>
#include <kernel/memory/phy_mem.h>
// Basic contans and macros
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12) //EVERY CHUNK SIZE IS A PAGE SIZE, IS 4096 BUYTES
#define  MAX(x, y) ((x) > (y)? (x) : (y))

//Pack size and allocated size in to a 4 byte
#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int*)(p))
#define PUT(p, val) (*(unsigned int*)(p) = (val))

//Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

//get the head and get the foot block
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(BP) ((char*)(bp) - WSIZE + GET_SIZE(HDRP(bp)) - WSIZE)

//GET NEX OR PRE BLOCK BP
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

//fllowing is funtion declaration
/**
 * @brief heap memeroy init
 * 
 * @param void 
 */
void mem_init();

/**
 * @brief head mem increase
 * 
 * @param increase size
 */
void* mem_sbrk(int incrs);

/**
 * @brief allocted init
 * 
 * @param void
 */
int malloc_init();

/**
 * @brief free the bp mem
 * 
 * @param bp pointer
 */
void malloc_free(void* bp);

/**
 * @brief kernel malloc
 * 
 * @param malloc size
 */
void* k_malloc(size_t size);


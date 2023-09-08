#include <kernel/memory/malloc.h>
//declar some private variables
static char* heap_listp;
static char* mem_heap; //points to first bytes of heap
static char* mem_brk;  //points to last bytes of heap plus 1
static char* mem_max_addr; // max legal heap addr plus 1
extern uint32_t __heap_start;
//some static functions
/**
 * @brief extend heap
 * 
 * @param extend size
 */
static void* extend_heap(size_t words)
{
    char* bp;
    size_t size;

    //align
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;

    //initialize free block header/footer and the epilogue header
    PUT(HDRP(bp), PACK(size, 0)); // free block header
    PUT(FTRP(bp), PACK(size, 0)); // free block foot
    PUT(HDPR(NEXT_BLKP(bp)), PACK(0, 1)); //NEW Epilogue header

    //Coalesce if previous block is free
    return coalesce(bp);
}

/**
 * @brief Coalesce free blk
 * 
 * @param Coalesce bp pointer
 */
static void* coalesce(void* bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc)
        return bp;

    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/**
 * @brief find free blk
 * 
 * @param find size
 */
static void* find_fit(size_t asize)// we just traverse heap
{
    void* bp = heap_listp;
    while(GET_ALLOC(HDRP(bp)) || (GET_SIZE(HDRP(bp)) < asize))
    {
        if(GET_SIZE(bp) != 0 && GET_ALLOC(bp) != 1)
            bp = NEXT_BLKP(bp);
        else
            return NULL;
    }
    return bp;
}

/**
 * @brief place the blk
 * 
 * @param blk,size
 */
static void place(void* bp, size_t asize)
{
    size_t bp_allsize = GET_SIZE(HDRP(bp));
    size_t remain_size = bp_allsize - asize;

    if(remain_size >= 2*DSIZE)
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(remain_size, 0));
    }else{
        PUT(HDRP(bp), PACK(bp_allsize, 1));
        PUT(FTRP(bp), PACK(bp_allsize, 1));
    }
}

//following is head file function

void mem_init()
{
    mem_heap = &__heap_start;
    //mem_heap = (char*)Malloc(MAX_HEAP);
    mem_brk = (char*)mem_heap;
    //mem_max_addr = (char*)(mem_heap + MAX_HEAP);
    mem_max_addr = (char*)K_STACK_START;
    
    if(vmm_alloc_page(mem_brk, PG_PREM_RW, PG_PREM_RW) == NULL)
        return;
    malloc_init();
}

void* mem_sbrk(int incr)
{
    char* old = mem_brk;
    if((incr < 0) || ((mem_brk + incr) > mem_max_addr))
    {
        return (void*)-1;
    }
    mem_brk += incr;
    return (void*)old;
}

int malloc_init()
{
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void*) - 1)
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));
    heap_listp += (2*WSIZE);

    //extend free heap with free blk og CHUNKSIZE size
    if(extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

void malloc_free(void* bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

void* k_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char* bp;

    if(size == 0)
        return NULL;
    
    //THE MIN blk is 2*DSIZE, ONE DSIZE means head and foot, one is align
    if(size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + DSIZE) + (DSIZE - 1)) / DSIZE; // Round up 
    //search the free list of fit
    if((bp = find_fit(asize)) != NULL)
    {
        place(bp, asize);
        return bp;
    }

    //no fit found try to extend head mem
    extendsize = MAX(asize, CHUNKSIZE);
    if((bp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}
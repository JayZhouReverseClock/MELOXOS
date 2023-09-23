#ifndef COMMON_H
#define COMMON_H
#define bool	_Bool
#define true	1
#define false	0
#define KSTACK_SIZE             (64 << 10)
#define KSTACK_START            ((0xFFBFFFFFU - KSTACK_SIZE) + 1)
#define KSTACK_TOP              0xffbffff0
#define HIGHER_HLF_BASE         0xC0000000
#define MEM_1MB                 0x100000

#define assert(cond)                                  \
    if (!(cond)) {                                    \
        __assert_fail(#cond, __FILE__, __LINE__);     \
    }


inline static void spin() {
    while(1);
}

#include <stddef.h>
// From Linux kernel v2.6.0 <kernel.h:194>
/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif
#ifndef COMMON_H
#define COMMON_H
#define bool	_Bool
#define true	1
#define false	0

#define assert(cond)                                  \
    if (!(cond)) {                                    \
        __assert_fail(#cond, __FILE__, __LINE__);     \
    }


inline static void spin() {
    while(1);
}

#endif
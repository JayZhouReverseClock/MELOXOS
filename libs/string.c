#include <libs/mstring.h>
#include <stdint.h>

void* memset(void* ptr, int value, size_t num)
{
    uint8_t* c_ptr = (uint8_t*)ptr;
    for (size_t i = 0; i < num; i++) {
        *(c_ptr + i) = (uint8_t)value;
    }
    return ptr;
}
#ifndef TSS_MELOX_H
#define TSS_MELOX_H
#include <stdint.h>
#include <common.h>

struct x86_tss {
    uint32_t link;
    uint32_t esp0;
    uint16_t ss0;
    uint8_t __padding[94];
} __attribute__((packed));

void tss_update_esp(uint32_t esp0);

#endif
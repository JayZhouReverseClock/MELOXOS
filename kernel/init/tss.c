#include <init/tss.h>

struct x86_tss _tss = {
    .link = 0,
    .esp0 = KSTACK_START,
    .ss0  = KDATA_SEG
};

void tss_update_esp(uint32_t esp0) {
    _tss.esp0 = esp0;
}
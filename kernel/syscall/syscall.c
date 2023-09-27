#include <kernel/process/peocess.h>
#include <syscall/syscall.h>

extern void syscall_handlr(isr_param* param);

void syscall_install() {
    intr_subscribe(MELOX_SYS_CALL, syscall_handlr);
}

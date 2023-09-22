#include <common.h>
#include <libs/mstdio.h>

static char buffer[1024];

void __assert_fail(const char* expr, const char* file, unsigned int line) {
    kprintf(buffer, "%s (%s:%u)", expr, file, line);

    // Here we load the buffer's address into %edi ("D" constraint)
    //  This is a convention we made that the LUNAIX_SYS_PANIC syscall will
    //  print the panic message passed via %edi. (see kernel/asm/x86/interrupts.c)
    asm(
        "int %0"
        ::"i"(32), "D"(buffer)
    );

    spin();     // never reach
}
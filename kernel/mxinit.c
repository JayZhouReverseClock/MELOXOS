#include <libs/mstdio.h>
#include <libs/mstdlib.h>
#include <vga/vga.h>
#include <stdint.h>
#include <kernel/process/peocess.h>
#include <kernel/process/sched.h>
#include <syscall/meloxstd.h>
#include <syscall/syscall.h>
#include <kernel/cpu/io.h>
//#define FORK_BOMB_DEMO
void _mxinit_main()
{
#ifdef FORK_BOMB_DEMO
    // fork炸弹
    for (;;) {
        pid_t p;
        if ((p = fork())) {
            kprintf("Pinkie Pie #%d: FUN!\n", p);
        }
    }
#endif

#ifdef WAIT_DEMO
    // 测试wait
    kprintf("I am parent, going to fork my child and wait.\n");
    if (!fork()) {
        kprintf("I am child, going to sleep for 2 seconds\n");
        sleep(2);
        kprintf("I am child, I am about to terminated\n");
        _exit(1);
    }
    int status;
    pid_t child = wait(&status);
    kprintf(
      "I am parent, my child (%d) terminated with code: %d.\n", child, status);
#endif
    // for(int i = 0; i < 10000; i++);

    // for (size_t i = 0; i < 10; i++) {
    //     pid_t pid = 0;
    //     if (!(pid = fork())) {
    //         for(int i = 0; i < 10000; i++);
    //         if (i == 3) {
    //             i = *(int*)0xdeadc0de; // seg fault!
    //         }
    //         vga_put_char('0' + i);
    //         vga_put_char('\n');
    //         _exit(0);
    //     }
    //     kprintf("Forked %d\n", pid);
    // }

    // char buf[64];
    kprintf("proc 1!");
    //io_outb(0x20, 0x20);
    //io_outb(0xa0, 0x20);//EOI end the int
    while(1);
}
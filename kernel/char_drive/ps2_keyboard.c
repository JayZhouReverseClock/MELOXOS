#include <kernel/char_drive/ps2_keyboard.h>
#include <kernel/cpu/cpu.h>
#include <init/idt.h>
#include <vga/vga.h>

void init_8259()
{
    asm volatile("cli");

    //ICW1
    io_outb(0x20, 0x11);
    cpu_delay();
    io_outb(0xa0, 0x11);
    cpu_delay();

    //ICW2
    io_outb(0x21, 0x20);
    cpu_delay();
    io_outb(0xa1, 0x28);
    cpu_delay();

    //ICW3
    io_outb(0x21, 0x04);
    cpu_delay();
    io_outb(0xa1, 0x02);
    cpu_delay();

    //ICW4
    io_outb(0x21, 0x01);
    cpu_delay();
    io_outb(0xa1, 0x01);
    cpu_delay();

    //OCW1
    io_outb(0x21, 0xff);
    cpu_delay();
    io_outb(0xa1, 0xff);

    asm volatile("sti");
}


static void ps2_post_cmd(uint8_t port, char cmd, uint16_t arg) {
    char result;
    // 等待PS/2输入缓冲区清空，这样我们才可以写入命令
    while((result = io_inb(PS2_PORT_CTRL_STATUS)) & PS2_STATUS_IFULL);

    io_outb(port, cmd);
    if (!(arg & PS2_NO_ARG)) {
        // 所有参数一律通过0x60传入。
        io_outb(PS2_PORT_ENC_CMDREG, (uint8_t)(arg & 0x00ff));
    }
}

static uint8_t ps2_issue_cmd(char cmd, uint16_t arg) {
    ps2_post_cmd(PS2_PORT_CTRL_CMDREG, cmd, arg);

    char result;
    
    // 等待PS/2控制器返回。通过轮询（polling）状态寄存器的 bit 0
    // 如置位，则表明返回代码此时就在 0x60 IO口上等待读取。
    while(!((result = io_inb(PS2_PORT_CTRL_STATUS)) & PS2_STATUS_OFULL));

    return io_inb(PS2_PORT_ENC_CMDREG);
}

static uint8_t ps2_issue_dev_cmd(char cmd, uint16_t arg) {
    ps2_post_cmd(PS2_PORT_ENC_CMDREG, cmd, arg);

    char result;
    
    // 等待PS/2控制器返回。通过轮询（polling）状态寄存器的 bit 0
    // 如置位，则表明返回代码此时就在 0x60 IO口上等待读取。
    while(!((result = io_inb(PS2_PORT_CTRL_STATUS)) & PS2_STATUS_OFULL));

    return io_inb(PS2_PORT_ENC_CMDREG);
}

void init_ps2k()
{
    asm volatile("cli");

     // 1、禁用任何的PS/2设备
    ps2_post_cmd(PS2_PORT_CTRL_CMDREG, PS2_CMD_PORT1_DISABLE, PS2_NO_ARG);
    ps2_post_cmd(PS2_PORT_CTRL_CMDREG, PS2_CMD_PORT2_DISABLE, PS2_NO_ARG);
    
    // 2、清空控制器缓冲区
    io_inb(PS2_PORT_ENC_DATA);

    char result;

    // 3、屏蔽所有PS/2设备（端口1&2）IRQ，并且禁用键盘键码转换功能
    result = ps2_issue_cmd(PS2_CMD_READ_CFG, PS2_NO_ARG);
    result = result & ~(PS2_CFG_P1INT | PS2_CFG_P2INT | PS2_CFG_TRANSLATION);
    ps2_post_cmd(PS2_PORT_CTRL_CMDREG, PS2_CMD_WRITE_CFG, result);

    // 4、控制器自检
    result = ps2_issue_cmd(PS2_CMD_SELFTEST, PS2_NO_ARG);
    if (result != PS2_RESULT_TEST_OK) {
        kprintf("Controller self-test failed.");
        goto done;
    }

    // 5、设备自检（端口1自检，通常是我们的键盘）
    result = ps2_issue_cmd(PS2_CMD_SELFTEST_PORT1, PS2_NO_ARG);
    if (result != 0) {
        kprintf("Interface test on port 1 failed.");
        goto done;
    }

    // 6、开启位于端口1的 IRQ，并启用端口1。不用理会端口2，那儿一般是鼠标。
    ps2_post_cmd(PS2_PORT_CTRL_CMDREG, PS2_CMD_PORT1_ENABLE, PS2_NO_ARG);
    result = ps2_issue_cmd(PS2_CMD_READ_CFG, PS2_NO_ARG);
    result = result | PS2_CFG_P1INT;
    ps2_post_cmd(PS2_PORT_CTRL_CMDREG, PS2_CMD_WRITE_CFG, result);

    // 至此，PS/2控制器和设备已完成初始化，可以正常使用。

done:
    asm volatile("sti");
}



void init_keyboard()
{
    init_8259();

    uint8_t result = io_inb(0x21);
    io_outb(0x21, result & 0xfd);
    cpu_delay();
    result = io_inb(0x21);

    init_ps2k();
    
    _set_idt_entry(0x21, 0x08, &handle_keywords, 0);
}

void handle_keywords()
{
    asm volatile("cli");
    char key;
    io_outb(0x61, 0x20);
    key = io_inb(0x60);

    vga_put_char(key);

    io_outb(0x20, 0x20);
    io_outb(0x20, 0x20);
    asm volatile("sti");
}
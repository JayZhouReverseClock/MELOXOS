#include <kernel/char_drive/ps2_keyboard.h>
#include <kernel/cpu/cpu.h>
#include <init/idt.h>
#include <vga/vga.h>

//#define SCANCODE1 1
#define SCANCODE3 3

//use those varible to record status, ext_scancode use to record makecode is use 0xeo begin
static uint8_t crtl_status, shift_status, alt_status, caps_lock_status, ext_scancode; 
#ifdef SCANCODE1
static char key_map[][2] = {
     //扫描码without shift 
/* ---------------------------------- */ 
/* OxOO */ {0,   0},
/* 0x01 */ {esc, esc},
/* Ox02 */ {'1',    '!'}, 
/* Ox03 */ {'2',    '@'},
/* Ox04 */ {'3',    '#'}, 
/* Ox05 */ {'4',    '$'},
/* Ox06 */ {'5',    '%'},
/* Ox07 */ {'6',    '^'},
/* Ox08 */ {'7',    '&'},
/* Ox09 */ {'8',    '*'},
/* OxOA */ {'9',    '('},
/* OxOB */ {'0',    ')'},
/* OxOC */ {'-',    '_'},
/* OxOD */ {'=',    '+'},
/* OxOE */ {backspace, backspace},
/* OxOF */ {tab, tab},
/* Ox1O */ {'q',    'Q'},
/* Ox11 */ {'w',    'W'},
/* Ox12 */ {'e',    'E'},
/* Ox13 */ {'r',    'R'}, 
/* Ox14 */ {'t',    'T'}, 
/* Ox15 */ {'y',    'Y'},
/* Ox16 */ {'u',    'U'},
/* Ox17 */ {'i',    'I'},
/* 0x18 */ {'o',    'O'},
/* Ox19 */ {'p',    'P'}, 
/* Ox1A */ {'[',    '{'},
/* Ox1B */ {']',    '}'}, 
/* Ox1C */ {enter,    enter},
/* Ox1D */ {ctrl_l_char,    ctrl_l_char},
/* Ox1E */ {'a',    'A'},
/* Ox1F */ {'s',    'S'},
/* Ox20 */ {'d',    'D'},
/* Ox21 */ {'f',    'F'},
/* Ox22 */ {'g',    'G'},
/* Ox23 */ {'h',    'H'},
/* Ox24 */ {'j',    'J'},
/* Ox25 */ {'k',    'K'},
/* Ox26 */ {'l',    'L'},
/* Ox27 */ {';',    ':'},
/* Ox28 */ {'\'',    '"'},
/* Ox29 */ {'`',    '~'},
/* Ox2A */ {shift_l_char,    shift_l_char}, 
/* Ox2B */ {'\\',    '|'}, 
/* Ox2C */ {'z',    'Z'},
/* Ox2D */ {'x',    'X'},
/* Ox2E */ {'c',    'C'},
/* Ox2F */ {'v',    'V'},
/* Ox30 */ {'b',    'B'},
/* Ox31 */ {'n',    'N'},
/* Ox32 */ {'m',    'M'},
/* Ox33 */ {',',    '<'},
/* Ox34 */ {'.',    '>'},
/* Ox35 */ {'/',    '?'},
/* Ox36 */ {shift_r_char,    shift_r_char},
/* Ox37 */ {'*',    '*'},
/* Ox38 */ {alt_l_char,    alt_l_char},
/* Ox39 */ {' ',    ' '},
/* Ox3A */ {caps_lock_char,    caps_lock_char},
//other keys tmp do not hand
};
#endif

#ifdef SCANCODE3
static char key_map[][2] = {
     //扫描码without shift 
/* ---------------------------------- */ 
/* OxOO */ {0,   0},
/* 0x01 */ {0, 0},
/* Ox02 */ {backspace,    backspace}, 
/* Ox03 */ {backspace,    backspace},
/* Ox04 */ {backspace,    backspace}, 
/* Ox05 */ {backspace,    backspace},
/* Ox06 */ {backspace,    backspace},
/* Ox07 */ {backspace,    backspace},
/* Ox08 */ {esc,    esc},
/* Ox09 */ {'8',    '*'},
/* OxOA */ {'9',    '('},
/* OxOB */ {'0',    ')'},
/* OxOC */ {'-',    '_'},
/* OxOD */ {tab,    tab},
/* OxOE */ {'`',    '~'},
/* OxOF */ {tab, tab},
/* Ox1O */ {'q',    'Q'},
/* Ox11 */ {'w',    'W'},
/* Ox12 */ {shift_l_char,    shift_l_char},
/* Ox13 */ {ctrl_l_char,    ctrl_l_char}, 
/* Ox14 */ {caps_lock_char,    caps_lock_char}, 
/* Ox15 */ {'q',    'Q'},
/* Ox16 */ {'1',    '!'},
/* Ox17 */ {'i',    'I'},
/* 0x18 */ {'o',    'O'},
/* Ox19 */ {alt_l_char,    alt_l_char}, 
/* Ox1A */ {'z',    'Z'},
/* Ox1B */ {'s',    'S'}, 
/* Ox1C */ {'a',    'A'},
/* Ox1D */ {'w',    'W'},
/* Ox1E */ {'2',    '@'},
/* Ox1F */ {'s',    'S'},
/* Ox20 */ {'d',    'D'},
/* Ox21 */ {'c',    'C'},
/* Ox22 */ {'x',    'X'},
/* Ox23 */ {'h',    'H'},
/* Ox24 */ {'e',    'E'},
/* Ox25 */ {'4',    '$'},
/* Ox26 */ {'3',    '#'},
/* Ox27 */ {';',    ':'},
/* Ox28 */ {'\'',    '"'},
/* Ox29 */ {(char)0x20,    (char)0x20},
/* Ox2A */ {'v',    'V'}, 
/* Ox2B */ {'f',    'F'}, 
/* Ox2C */ {'t',    'T'},
/* Ox2D */ {'r',    'R'},
/* Ox2E */ {'5',    '%'},
/* Ox2F */ {'v',    'V'},
/* Ox30 */ {'b',    'B'},
/* Ox31 */ {'n',    'N'},
/* Ox32 */ {'b',    'B'},
/* Ox33 */ {'h',    'H'},
/* Ox34 */ {'g',    'G'},
/* Ox35 */ {'y',    'Y'},
/* Ox36 */ {'6',    '^'},
/* Ox37 */ {'*',    '*'},
/* Ox38 */ {alt_l_char,    alt_l_char},
/* Ox39 */ {alt_r_char,    alt_r_char},
/* Ox3A */ {'m',    'M'},
/* Ox3B */ {'j',    'J'}, 
/* Ox3C */ {'u',    'U'},
/* Ox3D */ {'7',    '&'},
/* Ox3E */ {'8',    '*'},
/* Ox3F */ {'v',    'V'},
/* Ox40 */ {'b',    'B'},
/* Ox41 */ {'n',    'N'},
/* Ox42 */ {'k',    'K'},
/* Ox43 */ {'i',    'I'},
/* Ox44 */ {'o',    'O'},
/* Ox45 */ {'y',    'Y'},
/* Ox46 */ {'9',    '('}, 
/* Ox47 */ {';',    ':'},
/* Ox48 */ {'\'',    '"'},
/* Ox49 */ {'`',    '~'},
/* Ox4A */ {shift_l_char,    shift_l_char}, 
/* Ox4B */ {'l',    'L'}, 
/* Ox4C */ {'t',    'T'},
/* Ox4D */ {'p',    'P'},
/* Ox4E */ {'-',    '_'},
/* Ox4F */ {'v',    'V'},
/* Ox50 */ {'b',    'B'},
/* Ox51 */ {'n',    'N'},
/* Ox52 */ {'k',    'K'},
/* Ox53 */ {'i',    'I'},
/* Ox54 */ {'o',    'O'},
/* Ox55 */ {'=',    '+'},
/* Ox56 */ {'9',    '('}, 
/* Ox57 */ {';',    ':'},
/* Ox58 */ {ctrl_r_char,    ctrl_r_char},
/* Ox59 */ {shift_r_char,    shift_r_char},
/* Ox5A */ {enter,    enter},
/* Ox5B */ {'j',    'J'}, 
/* Ox5C */ {'u',    'U'},
/* Ox5D */ {'7',    '&'},
/* Ox5E */ {'8',    '*'},
/* Ox5F */ {'v',    'V'},
/* Ox60 */ {'b',    'B'},
/* Ox61 */ {'n',    'N'},
/* Ox62 */ {'k',    'K'},
/* Ox63 */ {'i',    'I'},
/* Ox64 */ {'o',    'O'},
/* Ox65 */ {'y',    'Y'},
/* Ox66 */ {backspace,    backspace}, 
/* Ox67 */ {';',    ':'},
/* Ox68 */ {'\'',    '"'},
/* Ox69 */ {'`',    '~'},
//other keys tmp do not hand
};
#endif

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

static void handle_keywords()
{
    crtl_status, shift_status, alt_status, caps_lock_status, ext_scancode = 0;
    asm volatile("cli");
    io_outb(0x61, 0x20);
    uint8_t ctrl_down_last = crtl_status;
    uint8_t shift_down_last = shift_status;
    uint8_t caps_lock_last = caps_lock_status;

    uint8_t break_code;
    uint16_t scancode = io_inb(0x60);
    
    if(scancode == 0xe0){
        ext_scancode = 1;
        io_outb(0x20, 0x20);
        io_outb(0x20, 0x20);
        asm volatile("sti");
        return;
    }
    if(ext_scancode){
        scancode = ((0xe000) | scancode);
        ext_scancode = 0;
    }

    break_code = ((0x0080 & scancode) != 0);

    if(break_code)//tan qi ma
    {
        uint16_t make_code = (scancode & 0xff7f);

        if(make_code == crtl_l_make || make_code == crtl_r_make)
        {
            crtl_status = 0;
        }else if(make_code == shift_l_make || make_code == shift_r_make){
            shift_status = 0;
        }else if(make_code == alt_l_make || make_code == alt_r_make){
            alt_status = 0;
        }
        io_outb(0x20, 0x20);
        io_outb(0x20, 0x20);
        asm volatile("sti");
        return;
    }//if is common word, hand
    else if((scancode > 0x00 && scancode < 0x3b) ||\
            (scancode == alt_r_make) || \
            (scancode == crtl_r_make)){
        uint8_t shift = 0;
        if((scancode < 0x0e) || (scancode == 0x29) || \
           (scancode == 0x1a) || (scancode == 0x1b) ||\
           (scancode == 0x2b) || (scancode == 0x27) ||\
           (scancode == 0x28) || (scancode == 0x33) ||\
           (scancode == 0x34) || (scancode == 0x35)){
            if(shift_down_last)
                shift = 1;
            else{
                if(shift_down_last && caps_lock_last)
                    shift = 0;
                else if(shift_down_last || caps_lock_last)
                    shift = 1;
                else
                    shift = 0;
                }
           }
        uint8_t index = (scancode &= 0x00ff);
        char cur_char = key_map[index][shift];

        if(cur_char){
            vga_put_char(cur_char);
            io_outb(0x20, 0x20);
            io_outb(0x20, 0x20);
            asm volatile("sti");
            return;
        }

        if(scancode == crtl_l_make || scancode == crtl_r_make)
            crtl_status = 1;
        else if(scancode == shift_l_make || scancode == shift_r_make)
            shift_status = 1;
        else if(scancode == caps_lock_make)
            caps_lock_status = ~caps_lock_status;
    }else{
        vga_put_char((char)(scancode && 0x00ff));
        io_outb(0x20, 0x20);
        io_outb(0x20, 0x20);
        asm volatile("sti");
    }
}
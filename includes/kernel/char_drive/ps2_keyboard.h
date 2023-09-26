#include <kernel/cpu/io.h>

#define PS2_PORT_ENC_DATA 0x60
#define PS2_PORT_ENC_CMDREG 0x60
#define PS2_PORT_CTRL_STATUS 0x64
#define PS2_PORT_CTRL_CMDREG 0x64

#define PS2_STATUS_OFULL   0x1
#define PS2_STATUS_IFULL   0x2

#define PS2_RESULT_ACK  0xfa
#define PS2_RESULT_NAK  0xfe    //resend
#define PS2_RESULT_ECHO 0xee    
#define PS2_RESULT_TEST_OK 0x55    

// PS/2 keyboard device related commands
#define PS2_KBD_CMD_SETLED  0xed
#define PS2_KBD_CMD_ECHO  0xee
#define PS2_KBD_CMD_SCANCODE_SET  0xf0
#define PS2_KBD_CMD_IDENTIFY  0xf2
#define PS2_KBD_CMD_SCAN_ENABLE  0xf4
#define PS2_KBD_CMD_SCAN_DISABLE  0xf5

// PS/2 *controller* related commands
#define PS2_CMD_PORT1_DISABLE  0xad
#define PS2_CMD_PORT1_ENABLE  0xae
#define PS2_CMD_PORT2_DISABLE  0xa7
#define PS2_CMD_PORT2_ENABLE  0xa8
#define PS2_CMD_SELFTEST      0xaa
#define PS2_CMD_SELFTEST_PORT1      0xab

#define PS2_CMD_READ_CFG       0x20
#define PS2_CMD_WRITE_CFG       0x60

#define PS2_CFG_P1INT             0x1
#define PS2_CFG_P2INT             0x2
#define PS2_CFG_TRANSLATION       0x40

#define PS2_DELAY       1000

#define PS2_CMD_QUEUE_SIZE 8
#define PS2_KBD_RECV_BUFFER_SIZE 8

#define PS2_NO_ARG 0xff00

#define KBD_BUF_PORT Ox60 //键盘 buffer 寄存器端口号为 Ox60
 
 // 用转义字符定义部分控制字符
#define esc  '\033' //八进制表示字符，也可以用十六进制’＼xlb
#define backspace '\b' 
#define tab '\t' 
#define enter '\r' 
#define delete '\177' //八进制表示字符，十六进制为’＼x7f
 
//以上不可见字符一律定义为 •I 
#define char_invisible   0 
#define ctrl_l_char     char_invisible 
#define ctrl_r_char     char_invisible 
#define shift_l_char    char_invisible 
#define shift_r_char    char_invisible 
#define alt_l_char      char_invisible
#define alt_r_char      char_invisible 
#define caps_lock_char  char_invisible 
 
//定义控制字符的通码和断码舍
#define shift_l_make    0x2a
#define shift_r_make    0x36
#define alt_l_make      0x38
#define alt_r_make      0xe038
#define alt_r_break     0xe0b8
#define crtl_l_make     0x1d
#define crtl_r_make     0xe01d
#define crtl_r_break    0xe09d
#define caps_lock_make  0x3a

void init_8259();
void init_ps2k();
static void handle_keywords();
void init_keyboard();


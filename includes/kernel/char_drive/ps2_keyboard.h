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

void init_8259();
void init_ps2k();
void handle_keywords();
void init_keyboard();


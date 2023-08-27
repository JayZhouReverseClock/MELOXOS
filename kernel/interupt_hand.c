#include <kernel/interupt_hand.h>
#include <vga/vga.h>
void _divide_err()
{
    vga_set_show(1,VGA_COLOR_BLACK,VGA_COLOR_BLUE);
    kprintf("this is divide err\n");
}
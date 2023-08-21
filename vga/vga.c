#include <vga/vga.h>

vga_attr theme_color = VGA_COLOR_BLACK;

int YPOS = 0;
int XPOS = 0;

vga_attr *video = (vga_attr *) VIDEO;

void vga_put_char(char c)
{
    if(c == '\n')
    {
    newline:
        XPOS = 0;
        YPOS++;
    }else if(c == '\r')
        XPOS = 0;
    else{
        *(video + XPOS + YPOS * WIDTH) = (theme_color | c);
    }

    XPOS++;
    if (XPOS >= WIDTH)
        goto newline;
}

void vga_set_show(vga_attr flashing, vga_attr backgroud, vga_attr frontcolor)
{
    theme_color = (flashing << 7 | backgroud << 4 | frontcolor) << 8;
}

void vga_put_str(char* str)
{
    while (*str != '\0')
    {
        vga_put_char(*str);
        str++;
    }
}

void vga_clear()
{
  
    for(int i = 0; i < WIDTH * HIGHT; i++)
        *(video + i) = theme_color;

    XPOS = 0;
    YPOS = 0;
}
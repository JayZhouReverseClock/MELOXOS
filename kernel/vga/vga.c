#include <vga/vga.h>

vga_attr theme_color = VGA_COLOR_BLACK;

int YPOS = 0;
int XPOS = 0;

vga_attr *video;

void init_vga_buffer()
{
    video = (vga_attr *) VIDEO;
}

void set_vga_buffer(void* vga_buf)
{
    video = (vga_attr*)vga_buf;
}

void vga_put_char(char c)
{
    if(c == '\n')
    {
        YPOS++;
        XPOS = 0;
    }else if(c == '\r')
        XPOS = 0;
    else{
        *(video + XPOS + YPOS * WIDTH) = (theme_color | c);
        XPOS++;
    }
    if (XPOS >= WIDTH)
    {
        YPOS++;
        XPOS = 0;
    }
    if(YPOS >= HIGHT)
        vga_scrollup();
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

void vga_scrollup()
{
    unsigned int last_line = WIDTH * (HIGHT - 1);
    for (unsigned int i = 0; i < last_line; i++) {
        *(video + i) = *(video + WIDTH + i);
    }
    for (unsigned int i = 0; i < WIDTH; i++) {
        *(video + i + last_line) = theme_color;
    }
    YPOS = YPOS == 0 ? 0 : HIGHT - 1;
}
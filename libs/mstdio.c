#include <libs/mstdio.h>
#include <vga/vga.h>
void kprintf (const char *format, ...)
{
  char **arg = (char **) &format;
  int c;
  char buf[20];

  arg++;
  while ((c = *format++) != 0)
    {
      if (c != '%')
        vga_put_char (c);
      else
        {
          char *p, *p2;
          int pad0 = 0, pad = 0;
          
          c = *format++;
          if (c == '0')
            {
              pad0 = 1;
              c = *format++;
            }

          if (c >= '0' && c <= '9')
            {
              pad = c - '0';
              c = *format++;
            }

          switch (c)
            {
            case 'd':
            case 'u':
            case 'x':
              itoa (buf, c, *((int *) arg++));
              p = buf;
              goto string;
              break;

            case 's':
              p = *arg++;
              if (! p)
                p = "(null)";

            string:
              for (p2 = p; *p2; p2++);
              for (; p2 < p + pad; p2++)
                vga_put_char (pad0 ? '0' : ' ');
              while (*p)
                vga_put_char (*p++);
              break;

            default:
              vga_put_char (*((int *) arg++));
              break;
            }
        }
    }
}

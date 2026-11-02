#ifndef __VGA_H
#define __VGA_H

#define VGA_MEM ((volatile uint16_t *)0xB8000)
#define VGA_COLS 80
#define VGA_ROWS 25

void vga_putc(char c);

#endif // __VGA_H

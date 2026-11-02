#include <stdint.h>

#include "video/vga.h"

static unsigned vga_row, vga_col;
static const uint8_t vga_attr = 0x07;

static void vga_scroll(void)
{
    for (unsigned i = 0; i < VGA_COLS * (VGA_ROWS - 1); i++)
        VGA_MEM[i] = VGA_MEM[i + VGA_COLS];
    for (unsigned i = VGA_COLS * (VGA_ROWS - 1); i < VGA_COLS * VGA_ROWS; i++)
        VGA_MEM[i] = (vga_attr << 8) | ' ';
}

void vga_putc(char c)
{
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\b') {
        if (vga_col)
            VGA_MEM[vga_row * VGA_COLS + --vga_col] = (vga_attr << 8) | ' ';
    } else {
        VGA_MEM[vga_row * VGA_COLS + vga_col] = (vga_attr << 8) | (uint8_t)c;
        if (++vga_col == VGA_COLS) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row == VGA_ROWS) {
        vga_scroll();
        vga_row = VGA_ROWS - 1;
    }
}

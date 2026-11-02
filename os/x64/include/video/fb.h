#ifndef __FB_H
#define __FB_H

#include <stdint.h>
#include <stdbool.h>

#include "kernel/multiboot.h"

#define FB_COLOR_WHITE 0x00FFFFFF

#define GLYPH_MAX_ROWS 8
#define GLYPH_MAX_COLS 8

void fb_init(multiboot_info_t *mb);
void fb_putpixel(uint32_t x, uint32_t y, uint32_t color);
void draw_glyph(uint8_t *__counted_by(8) glyph, uint32_t x, uint32_t y, uint32_t color);
void fb_putc_rgb(uint8_t c);
void fb_putc(char c);
void fb_puts(const char *s);

#endif // __FB_H

#include <stdint.h>
#include <stdbool.h>

#include "ptrcheck.h"

#include "video/fb.h"
#include "video/vga.h"
#include "kernel/multiboot.h"
#include "fonts/font8x8_basic.h"
#include "mm/mm.h"
#include "mm/paging.h"
#include "util/util.h"

static uint32_t fb_pitch, fb_width, fb_height;
static uint64_t fb_size;
static volatile void *fb_addr;
static uint8_t fb_bpp, fb_type;
static bool fb_available;

// pixel cursor for rgb mode
static uint32_t fb_cur_x, fb_cur_y;

void fb_init(multiboot_info_t *mb)
{
    if (!(mb->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO)) {
        fb_available = false;
        return;
    }

    fb_pitch = mb->framebuffer_pitch;
    fb_width = mb->framebuffer_width;
    fb_height = mb->framebuffer_height;
    fb_size = (uint64_t) fb_height * fb_pitch;
    fb_addr = __unsafe_forge_bidi_indexable(volatile void *, mb->framebuffer_addr,
                                            fb_size);
    fb_bpp= mb->framebuffer_bpp;
    fb_type = mb->framebuffer_type;
    fb_available = true;

    // map fb_addr
    uint64_t fb_start = align_down((uint64_t) fb_addr, FOUR_K);
    uint64_t fb_end = align_up(fb_start + fb_size, FOUR_K);
    map_range(fb_start, fb_end);
    reload_cr3();

    fb_cur_x = 0;
    fb_cur_y = 0;
}

static
uint8_t *__counted_by(8) glyph_for(uint8_t code)
{
    if (code > 0x7f)
        code = 0x20; // fall back to the space glyph, index 0x20
    return font8x8_basic[code];
}

void fb_putpixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (x >= fb_width || y >= fb_height)
        return;

    uint64_t size = fb_size;
    volatile uint8_t *__sized_by(size) base =
        __unsafe_forge_bidi_indexable(volatile uint8_t *,
                                      (uint64_t) fb_addr, size);
    volatile uint8_t *row = base + (y * fb_pitch);

    if (fb_bpp == 32) {
        // times 4 bytes (32 bits per pixel)
        volatile uint32_t *p = (volatile uint32_t *)(row + x * 4);
        *p = color;
    } else if (fb_bpp == 24) {
        row[x*3 + 0] = color & 0xff;
        row[x*3 + 1] = (color >> 8) & 0xff;
        row[x*3 + 2] = (color >> 16) & 0xff;
    }
}

void draw_glyph(uint8_t *__counted_by(8) glyph, uint32_t x, uint32_t y, uint32_t color)
{
    for (unsigned row = 0; row < GLYPH_MAX_ROWS; row++)
        for (unsigned col = 0; col < GLYPH_MAX_COLS; col++)
            // is the pixel set? if it is, draw it
            if (glyph[row] & ( 0x01 << col ))
                fb_putpixel(x + col, y + row, color);
}

void fb_putc_rgb(uint8_t c)
{
    if (c == '\n') {
        fb_cur_x = 0;
        fb_cur_y += 8;
    } else if (c == '\r') {
        fb_cur_x = 0;
    } else {
        uint8_t *__counted_by(8) glyph = glyph_for(c);
        draw_glyph(glyph, fb_cur_x, fb_cur_y, FB_COLOR_WHITE);
        fb_cur_x += 8;
        if (fb_cur_x + 8 > fb_width) {
            // out off horizontal room, wrap to top; no scroll
            fb_cur_x = 0;
            fb_cur_y += 8;
        }
    }
    if (fb_cur_y + 8 > fb_height) {
        // out of vertical room, wrap to top
        fb_cur_x = 0;
        fb_cur_y = 0;
    }
}

void fb_putc(char c)
{
    if (!fb_available)
        return;

    switch (fb_type) {
        case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
            vga_putc(c);
            break;
        case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
            fb_putc_rgb((uint8_t) c);
            break;
        default:
            break;
    }
}

void fb_puts(const char *s)
{
    while (*s)
        fb_putc(*s++);
}

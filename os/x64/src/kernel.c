#include <stdint.h>

#include "irq/idt.h"
#include "cpu/tss.h"
#include "util/printf.h"
#include "mm/mm.h"
#include "kernel.h"

void init64()
{
    init_idt();
    init_tss();
    init_bump_allocator();
}

void kernel_main(void)
{
    init64();

    printf(CLEAR_SCREEN);
    printf( "  _   _                     __     __  ____       _                 \n"
            " | | | |_   _ _ __   ___ _ _\\ \\   / / |  _ \\  ___| |__  _   _  __ _ \n"
            " | |_| | | | | '_ \\ / _ \\ '__\\ \\ / /  | | | |/ _ \\ '_ \\| | | |/ _` |\n"
            " |  _  | |_| | |_) |  __/ |   \\ V /   | |_| |  __/ |_) | |_| | (_| |\n"
            " |_| |_|\\__, | .__/ \\___|_|    \\_/    |____/ \\___|_.__/ \\__,_|\\__, |\n"
            "        |___/|_|                                              |___/ \n");

    __asm__ volatile ("int3");

    printf("IDT handled the software interrupt successfully!\n");

    void *page = bump_alloc(10);
    printf("Got page from bump allocator: 0x%p\n", (uint64_t)page);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

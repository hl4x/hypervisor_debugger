#include <stdint.h>

#include "kernel/kernel.h"
#include "kernel/multiboot.h"
#include "irq/idt.h"
#include "cpu/tss.h"
#include "cpu/cpu.h"
#include "cpu/svm/svm.h"
#include "cpu/svm/host.h"
#include "util/printf.h"
#include "util/bug.h"
#include "mm/mm.h"
#include "mm/paging.h"

extern void guest_entry(void);
extern void guest_stack_top(void);

void init64(multiboot_info_t *mb)
{
    init_idt();
    init_tss();
    parse_multiboot_memory(mb);
    map_available_ranges();
}

static
void print_banner(void)
{
    printf(CLEAR_SCREEN);
    // https://emojicombos.com/ocean-ascii-art
    printf( " ⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⠇⡅⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⠧⡇⠀⠀⠒⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⠀⡤⡆⠦⠆⢀⠠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⠧⣷⣆⠅⢦⠀⠀⠀⠀⠀⠀⠀⠀⠠⠀⠈⠀⠀⠀⠀⠀⢤⣤⣆⢇⣶⣤⡤⡯⣦⣌⡡⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⠷⣿⣷⣆⣐⡆⠀⠀⠀⠀⢀⠤⠊⠀⠀⢀⣠⣾⢯⣦⣴⣜⣺⣾⣿⣤⠟⠋⣷⢛⡣⠭⠢⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⠯⣿⣷⢫⡯⠄⠀⠀⢀⠐⠁⠀⠀⠀⠠⣤⣿⣿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣙⣷⡗⢤⡤⠀⠈⣰⠶⡤⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⣩⣿⡏⠉⠉⠀⢠⡔⠁⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠑⣏⠶⡉⠖⣡⠂⣈⣤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⣮⣿⣧⣤⣤⠖⠁⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⢉⡻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠓⠈⠅⠈⠀⠀⠘⢒⣽⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⣿⡿⠛⠉⠀⠀⠀⣀⠔⢀⡴⣃⠀⠀⢀⠷⠲⡄⠸⠟⢋⣿⣿⣿⣿⣿⡇⠀⠀⠀⠐⠁⠀⠀⠂⠀⠀⠰⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⡆⣷⣆⡐⠶⠤⢤⣷⣀⣀⣩⢐⣟⣥⠜⣤⣀⣠⣤⠀⠈⠉⢀⣹⣿⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
            " ⢃⣿⣞⣫⡔⢆⡸⡿⣿⣿⣄⣰⣿⠁⢀⣛⠿⣻⣿⣿⣧⣬⣿⣿⣿⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⢀ \n"
            " ⢼⣿⣟⢿⣧⣾⣵⣷⣿⣿⣟⡿⢿⣶⣞⣍⡴⢿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⣠⠈⠀⢀⣀⣼ \n"
            " ⠋⣿⣟⡛⢿⣿⣿⣿⣿⣿⣭⣿⣿⣿⣿⣯⣽⣿⣿⣿⣿⠟⠛⠿⢽⣿⣿⣆⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡀⣀⢀⡠⣤⣤⣰⣿⠟⠁⠀⠀⡼⢾⣿ \n"
            " ⣻⣿⣟⣇⠈⣉⣯⠿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠃⠀⠀⠀⠀⠀⠻⣿⣿⣿⣿⣴⣶⣤⣤⣤⣤⣴⣴⣴⣶⣦⣦⣤⣦⣀⣦⣤⣶⣿⣿⣿⣿⣿⣿⣿⠿⠁⠀⠀⡀⣤⣬⣾⣿ \n"
            " ⡝⣿⣿⣇⣤⣶⣿⣷⣾⣭⡿⠻⢿⣿⣿⣿⣿⠿⠃⠀⠀⠀⠀⡄⠀⠀⠀⢊⡻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠋⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⢿⠟⢉⠀⡀⢤⣴⣿⣿⣿⠿⠻ \n"
            " ⡁⣻⣿⣿⣿⣿⣷⣿⣿⣿⣿⠾⣿⡿⠞⠁⠀⠀⠀⠀⠀⠔⠫⡅⠀⠀⠀⠀⠁⣀⠀⠈⠻⣿⣿⣿⣿⣻⢟⣁⣄⡄⣀⠙⠻⣿⣿⡿⠿⠛⡋⠕⠂⢀⣀⣄⣓⣳⢿⠟⢛⣩⠴⠈⠀ \n"
            " ⠂⡁⠈⠛⠛⠛⠛⠋⠁⠀⠈⠈⡀⠀⠀⠀⠀⢀⠘⠀⠀⠀⠆⠀⡀⡢⣀⣆⠄⠈⠨⢦⡀⣈⠙⠛⠿⢿⣿⣿⣿⣿⣿⡿⡿⠿⠟⠆⠒⠁⠀⢶⣾⠿⠟⠛⢉⣀⣠⡶⠚⠁⠀⠀⣠ \n"
            " ⠀⡇⡄⣀⡀⠀⠀⠀⠀⠀⠀⠀⢬⠠⠀⡀⠀⠋⠁⠀⡀⠀⠀⡀⠆⢱⣿⣿⣧⣧⣄⠛⣿⣞⣵⣤⣷⣄⠀⠀⠀⠐⠀⠀⠀⠀⠀⠈⠉⠁⠁⠀⠠⢤⣶⣾⣿⡿⠋⢀⣀⣰⣶⣾⣿ \n"
            " ⡀⡆⠀⡉⡁⢿⣉⢀⠀⣰⣷⣿⣟⠠⡽⢂⡀⡄⠀⠰⣖⢱⢖⢂⡆⠈⣿⣿⣿⣿⣿⣶⣄⡙⠻⢿⣿⣿⣷⣦⣀⠀⠠⣤⣀⡀⢈⣓⣶⣶⣿⣿⣿⣿⣿⠟⠉⠀⠀⠀⣉⣭⣽⣿⣿ \n"
            " ⡇⣯⣿⣿⣿⣾⣿⣿⣿⠿⠟⡡⢞⣹⠾⢻⣚⣛⢺⠞⢋⣭⣾⣧⡃⢄⡈⢿⣿⣿⣿⣿⣿⣿⣯⣿⣮⣽⣿⣿⣿⣿⣷⣬⣽⣿⣿⣿⣽⡿⣿⡿⠟⠋⢀⣀⣐⣺⣿⣿⣟⣫⣭⣿⣿ \n"
            " ⢳⣿⣿⣿⣿⣿⣿⣿⣿⣤⣿⣿⣿⣿⣿⣦⠒⠉⢁⡀⠀⣙⣛⢿⣷⣶⣅⠀⠙⠻⣿⣿⣿⣿⣟⡚⠛⠻⠞⠿⠿⡿⡿⠯⠁⠟⣊⠾⠝⢋⣁⣀⣤⣤⣿⣿⣿⡿⠿⠿⠻⠛⠻⠻⠿ \n"
            " ⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⣐⣾⡿⡟⢶⠾⢋⢹⠿⢿⣿⣿⣷⣦⡈⠙⠛⠿⠿⢿⣶⣶⣶⣶⣶⢶⠟⠚⠀⠁⠀⠀⠙⠛⠛⠛⠛⠛⠋⠉⠁⠀⠀⠀⠀⠀⢀⠀⠀ \n");
}

void scratch_test(void)
{
    __asm__ volatile ("int3");

    printf("IDT handled the software interrupt successfully!\n");

    uint8_t* mem = (uint8_t*)bump_alloc(0x10);
    printf("Got memory from bump allocator: 0x%p\n", (uint64_t)mem);
    for (uint8_t i =0; i < 0x10; i++) {
        mem[i] = 0xff;
    }
    memset(mem, 0x41, 10);
    for (size_t i = 0; i < 16; i++)
        printf("%02x ", mem[i]);
    printf("\n");
}

void kernel_main(multiboot_info_t *mb)
{
    init64(mb);
    print_banner();
    scratch_test();

    svm_host_t host = {
        .hsave = bump_alloc_aligned(FOUR_K, FOUR_K),
        .vmsave = bump_alloc_aligned(FOUR_K, FOUR_K),
        .iopm = bump_alloc_aligned(FOUR_K * 3, FOUR_K),
        .msrpm = bump_alloc_aligned(FOUR_K * 2, FOUR_K),
    };
    svm_host_init(&host);
    svm_enable(&host);

    vcpu_t vcpu = { 0 };
    svm_vcpu_init(&vcpu, &host, (uint64_t)guest_entry,
                  (uint64_t)guest_stack_top, read_cr3(),
                  host.has_nrips);

    for (;;) {
        __asm__ volatile ("hlt");
    }
}

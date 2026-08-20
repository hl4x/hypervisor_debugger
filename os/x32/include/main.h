#ifndef __MAIN_H
#define __MAIN_H

#define PORT 0x3f8 // COM1
#define CLEAR_SCREEN "\e[1;1H\e[2J"

extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);
extern void enter_long_mode(void);
extern void setup_pgtable_32(void);

extern uint64_t kernel32_reserverd_start;
extern uint64_t kernel32_reserverd_end;

#endif // __MAIN_H
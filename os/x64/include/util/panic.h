#ifndef __PANIC_H
#define __PANIC_H

__attribute__((noreturn)) void panic(const char *fmt, ...);

#endif // __PANIC_H
#ifndef __BUG_H
#define __BUG_H

#include "util/printf.h"
#include "util/panic.h"

#define BUG_ON(condition, fmt, ...) do { \
    if (condition) { \
        printf("Bug at %s:%d - ", __FILE__, __LINE__); \
        panic(fmt, ##__VA_ARGS__); \
    } \
} while (0)

#endif // __BUG_H

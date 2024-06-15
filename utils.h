#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <limits.h>
#include <complex.h>

void print_binary(void* bytes_, size_t size) {
    unsigned char *bytes, byte, bit, mask;
    int i, j;
    bytes = (unsigned char*)bytes_;
    for (i = size - 1; i >= 0; --i) {
        byte = bytes[i];
        for (j = 7; j >= 0; --j) {
            mask = (1 << j);
            bit = !!(byte & mask);
            putchar(bit + '0');
        }
        if (i > 0)
            putchar(' ');
    }
    putchar('\n');
}

#endif /* UTILS_H */
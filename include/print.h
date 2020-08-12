#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

int vprintf(const char* fmt, va_list args);
int printf(const char* fmt, ...);
int putchar(int chr);
int sprintf(char* str, const char* fmt, ...);

#endif /* ifdef PRINT_H */

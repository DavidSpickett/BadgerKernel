#ifndef PRINT_H
#define PRINT_H

#ifdef linux
#include <stdio.h>
#else
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

int vprintf(const char* fmt, va_list args);
int printf(const char* fmt, ...);
int sprintf(char* str, const char* fmt, ...);

#endif // !linux

#endif /* ifdef PRINT_H */

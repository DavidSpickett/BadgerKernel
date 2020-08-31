#ifndef COMMON_PRINT_H
#define COMMON_PRINT_H

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

int vprintf(const char* fmt, va_list args);
int printf(const char* fmt, ...);
int putchar(int chr);
int sprintf(char* str, const char* fmt, ...);

#define THREAD_NAME_SIZE 12
void format_thread_name(char* out, int tid, const char* name);

#endif /* ifdef COMMON_PRINT_H */

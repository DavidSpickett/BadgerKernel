#ifndef COMMON_PRINT_H
#define COMMON_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

int vprintf(const char* fmt, va_list args);
int printf(const char* fmt, ...);
int putchar(int chr);
int sprintf(char* str, const char* fmt, ...);

void format_thread_name(char* out, int tid, const char* name);

enum TextColour { eReset, eYellow };
const char* text_colour(enum TextColour colour);

#ifdef __cplusplus
}
#endif

#endif /* ifdef COMMON_PRINT_H */

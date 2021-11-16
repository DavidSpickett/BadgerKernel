#ifndef COMMON_PRINT_H
#define COMMON_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/macros.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

BK_EXPORT int vprintf(const char* fmt, va_list args);
BK_EXPORT int printf(const char* fmt, ...);
BK_EXPORT int putchar(int chr);
BK_EXPORT int sprintf(char* str, const char* fmt, ...);

BK_EXPORT void format_thread_name(char* out, int tid, const char* name);

enum TextColour { eReset, eYellow };
BK_EXPORT const char* text_colour(enum TextColour colour);

#ifdef __cplusplus
}
#endif

#endif /* ifdef COMMON_PRINT_H */

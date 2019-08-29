#include <stdarg.h>
#include "print.h"

#ifdef linux

#include <stdio.h>

#else

int putchar(int c) {
  // This must be a an int write not a char
  volatile unsigned int * const UART0 = (unsigned int *)UART_BASE;
  *UART0 = (unsigned int)c;
  return c;
}

#endif

void print(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
      // Check for formatting arg
      if (*fmt == '%') {
        fmt++;

        if (*fmt == 's') {
          // string
          const char* str = va_arg(args, const char*);
          while (*str) {
            putchar(*str++);
          }
          fmt++;
        } else if (*fmt != '%') {
          __builtin_unreachable();
        }
      }

      putchar(*fmt++);
    }

    va_end(args);
}

// unsigned base 10 only
size_t uint_to_str(uint32_t num, char* out) {
  size_t len = 0;

  if (num) {
    char* start = out;

    uint32_t div = 10;
    while (num) {
      uint32_t digit = num % div;
      *out++ = ((char)digit)+48;
      len++;
      num /= div;
    }

    // Now reverse the digits
    --out;
    for (size_t idx = 0; idx != (len/2); ++idx) {
      char chr = start[idx];
      *(start+idx) = *(out-idx);
      *(out-idx) = chr;
    }
  } else {
    *out = '0';
    len = 1;
  }

  // Null terminate
  *(out+1) = '\0';

  return len;
}

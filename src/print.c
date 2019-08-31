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

void putstr(const char* str) {
  while (*str) {
    putchar(*str++);
  }
}

void print(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
      // Check for formatting arg
      if (*fmt == '%') {
        fmt++;

        switch (*fmt) {
          case 's':
          {
            // string
            const char* str = va_arg(args, const char*);
            putstr(str);
            fmt++;
            break;
          }
          case 'u':
          {
            // Unsigned decimal
            char num_str[6];
            unsigned num = va_arg(args, unsigned);
            uint_to_str(num, num_str);
            putstr(num_str);
            fmt++;
            break;
          }
          case '%':
            // Escaped char, print normally
            break;
          default:
            __builtin_unreachable();
            break;
        }
      }

      putchar(*fmt++);
    }

    va_end(args);
}

// unsigned base 10 only
size_t uint_to_str(unsigned num, char* out) {
  size_t len = 0;

  if (num) {
    char* start = out;

    unsigned div = 10;
    while (num) {
      unsigned digit = num % div;
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

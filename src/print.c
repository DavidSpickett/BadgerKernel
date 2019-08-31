#include "print.h"
#include <stdarg.h>

int putchar(int chr) {
  // This must be a an int write not a char
  volatile unsigned int * const UART0 = (unsigned int *)UART_BASE;
  *UART0 = (unsigned int)chr;
  return 1;
}

int putstr(const char* str) {
  int len = 0;
  while (*str) {
    len += putchar(*str++);
  }
  return len;
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

int printf(const char* fmt, ...)
{
    int len = 0;
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
      // Check for formatting arg
      if (*fmt == '%') {
        fmt++;

        switch (*fmt) {
          case 's': // string
          {
            len += putstr(va_arg(args, const char*));
            fmt++;
            break;
          }
          case 'u': // Unsigned decimal
          {
            char num_str[6];
            uint_to_str(va_arg(args, unsigned), num_str);
            len += putstr(num_str);
            fmt++;
            break;
          }
          case '%': // Escaped %
            len += putchar(*fmt++);
            break;
          default:
            __builtin_unreachable();
            break;
        }
      } else {
        // Any non formatting character
        len += putchar(*fmt++);
      }
    }

    va_end(args);
    return len;
}

int sprintf(char* str, const char* fmt, ...) {
   char* start = str;
   va_list args;
   va_start(args, fmt);

   while (*fmt) {
    if (*fmt == '%') {
      fmt++;

      if (*fmt == 'u') {
        unsigned num = va_arg(args, unsigned);
        str += uint_to_str(num, str);
        fmt++;
      } else if (*fmt == '%') {
        *str++ = *fmt++;
      } else {
        __builtin_unreachable();
      }
    } else {
      *str++ = *fmt++;
    }
   }

   *str = '\0';
   va_end(args);
   return str-start;
}

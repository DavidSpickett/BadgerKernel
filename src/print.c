#include "print.h"
#include <stdarg.h>
#include <stdint.h>

int putchar(int chr) {
  // This must be a an int write not a char
  volatile unsigned int* const UART0 = (unsigned int*)UART_BASE;
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

size_t uint_to_str(uint64_t num, char* out, unsigned base) {
  size_t len = 0;

  if (num) {
    char* start = out;

    while (num) {
      uint64_t digit = num % base;
      char new_char = digit >= 10 ? 65 : 48;
      new_char += digit % 10;
      *out++ = new_char;
      len++;
      num /= base;
    }

    // Now reverse the digits
    --out;
    for (size_t idx = 0; idx != (len / 2); ++idx) {
      char chr = start[idx];
      *(start + idx) = *(out - idx);
      *(out - idx) = chr;
    }
  } else {
    *out = '0';
    len = 1;
  }

  // Null terminate
  *(out + 1) = '\0';

  return len;
}

int printf(const char* fmt, ...) {
  int len = 0;
  va_list args;
  va_start(args, fmt);

  while (*fmt) {
    // Check for formatting arg
    if (*fmt == '%') {
      fmt++;

      // Ignore %l/%ll
      while (*fmt == 'l') {
        fmt++;
      }

      switch (*fmt) {
      case 's': // string
      {
        len += putstr(va_arg(args, const char*));
        fmt++;
        break;
      }
      case 'u': // Unsigned decimal
      case 'x': // unsigned hex
      case 'X':
      {
        unsigned base = *fmt == 'u' ? 10 : 16;
        // 64 bit hex plus null terminator
        char num_str[17];
        uint_to_str(va_arg(args, uint64_t), num_str, base);
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

      // Skip ls since we don't treat them differently yet
      while (*fmt == 'l') {
        fmt++;
      }

      if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X') {
        unsigned base = *fmt == 'u' ? 10 : 16;
        str += uint_to_str(va_arg(args, uint64_t), str, base);
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
  return str - start;
}

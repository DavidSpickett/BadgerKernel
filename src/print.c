#include "print.h"
#include "common/thread.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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
  va_list args;
  va_start(args, fmt);
  int printed = vprintf(fmt, args);
  va_end(args);
  return printed;
}

int vprintf(const char* fmt, va_list args) {
  int len = 0;

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
      case 'i': // Signed decimal
      {
        char int_str[17];
        int num = va_arg(args, int);
        if (num < 0) {
          len += putchar('-');
          num = abs(num);
        }
        uint_to_str(num, int_str, 10);
        len += putstr(int_str);
        fmt++;
        break;
      }
      case 'u': // Unsigned decimal
      case 'x': // unsigned hex
      {
        unsigned base = *fmt == 'u' ? 10 : 16;
        // 64 bit hex plus null terminator
        char num_str[17];
        uint_to_str(va_arg(args, size_t), num_str, base);
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

  return len;
}

int sprintf(char* str, const char* fmt, ...) {
  char* start = str;
  va_list args;
  va_start(args, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;

      if (*fmt == 'u' || *fmt == 'x' || *fmt == 'X') {
        unsigned base = *fmt == 'u' ? 10 : 16;
        str += uint_to_str(va_arg(args, size_t), str, base);
        fmt++;
      } else if (*fmt == '%') {
        *str++ = *fmt++;
      } else if (*fmt == 's') {
        const char* in_str = va_arg(args, const char*);
        strcpy(str, in_str);
        str += strlen(in_str);
        fmt++;
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

void format_thread_name(char* out, int tid,
                        const char* name) {
  // fill with spaces (no +1 as we'll terminate it later)
  for (size_t idx = 0; idx < THREAD_NAME_SIZE; ++idx) {
    out[idx] = ' ';
  }

  if (name == NULL) {
    // If the thread had a stack issue
    if (tid == INVALID_THREAD) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_SIZE - h_len;
      strncpy(&out[padding], hidden, h_len);
    } else {
      // Just show the ID number (assume max 999 threads)
      char idstr[4];
      int len = sprintf(idstr, "%u", tid);
      strcpy(&out[THREAD_NAME_SIZE - len], idstr);
    }
  } else {
    size_t name_len = strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_SIZE) {
      name_len = THREAD_NAME_SIZE;
    }

    size_t padding = THREAD_NAME_SIZE - name_len;
    strncpy(&out[padding], name, name_len);
  }

  out[THREAD_NAME_SIZE] = '\0';
}

#include "common/print.h"
#include "common/thread.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int putchar_n(int chr, int repeat) {
  if (repeat < 0) {
    return 0;
  }

  for (int i = 0; i < repeat; ++i) {
    // This must be a an int write not a char
    volatile unsigned int* const UART0 = (unsigned int*)UART_BASE;
    *UART0 = (unsigned int)chr;
  }
  return repeat;
}

int putchar(int chr) {
  return putchar_n(chr, 1);
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

static size_t pow(size_t base, size_t power) {
  if (!power) {
    return 1;
  }
  size_t ret = 1;
  for (; power > 1; --power) {
    ret *= base;
  }
  return ret;
}

static size_t consume_uint(const char** in) {
  if (!isdigit((unsigned char)**in)) {
    return (size_t)-1;
  }

  // Find end of number
  const char* end_ptr = (*in) + 1;
  while (isdigit((unsigned char)*end_ptr)) {
    ++end_ptr;
  }

  const char* begin_ptr = *in;
  size_t num = 0;

  size_t multiplier = pow(10, end_ptr - begin_ptr);

  for (; begin_ptr < end_ptr; ++begin_ptr) {
    num += ((*begin_ptr) - 48) * multiplier;
    multiplier /= 10;
  }

  // Consume the number
  *in = end_ptr;
  return num;
}

static va_list handle_format_char(int* out_len, const char** fmt_chr,
                                  va_list args) {
  // Save a bunch of * by making a copy now and
  // assigning back at the end
  const char* fmt = *fmt_chr;
  int len = 0;
  size_t padding_len = consume_uint(&fmt);
  if (padding_len == (size_t)-1) {
    padding_len = 0;
  }

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
      len += strlen(num_str);
      putchar_n('0', padding_len - len);
      putstr(num_str);
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

  *fmt_chr = fmt;
  *out_len += len;
  return args;
}

int vprintf(const char* fmt, va_list args) {
  int len = 0;

  while (*fmt) {
    // Check for formatting arg
    if (*fmt == '%') {
      fmt++;
      args = handle_format_char(&len, &fmt, args);
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

void format_thread_name(char* out, int tid, const char* name) {
  // fill with spaces (skip last, null terminator done later)
  for (size_t idx = 0; idx < THREAD_NAME_LEN; ++idx) {
    out[idx] = ' ';
  }

  if (!name || !strlen(name)) {
    // If the thread had a stack issue
    if (tid == INVALID_THREAD) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_LEN - h_len;
      strncpy(&out[padding], hidden, h_len);
    } else {
      // Just show the ID number (assume max 999 threads)
      char idstr[4];
      int len = sprintf(idstr, "%u", tid);
      strcpy(&out[THREAD_NAME_LEN - len], idstr);
    }
  } else {
    size_t name_len = strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_LEN) {
      name_len = THREAD_NAME_LEN;
    }

    size_t padding = THREAD_NAME_LEN - name_len;
    strncpy(&out[padding], name, name_len);
  }

  out[THREAD_NAME_LEN] = '\0';
}

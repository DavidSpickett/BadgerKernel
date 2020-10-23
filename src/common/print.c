#include "common/print.h"
#include "common/thread.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Think of this as a C++ struct where "this" is a PrintOutput*.
// Pass these to the "put" functions to manage where characters
// are written to.
typedef struct PrintOutput {
  // Write char to buf, then modify buf as appropriate
  void (*const writer)(struct PrintOutput*, int);
  // Char destination, may be modified by the writer
  char* buf;
} PrintOutput;

static void serial_writer(PrintOutput* output, int chr) {
  // This must be a an int write not a char
  volatile unsigned int* const UART0 = (unsigned int*)output->buf;
  *UART0 = (unsigned int)chr;
  // We do not modify buf here, serial port doesn't move
}

static void buffer_writer(PrintOutput* output, int chr) {
  *output->buf = (char)chr;
  // Writing to a memory buffer, so inc pointer
  ++output->buf;
}

// This is const but buffer writers cannot be const
// since we need to move the buffer pointer if writing
// to memory. So take a copy of this when you need to use it.
static const PrintOutput serial_output = {.writer = serial_writer,
                                          .buf = (char*)UART_BASE};

static int putchar_n_output(PrintOutput* output, int chr, unsigned int repeat) {
  for (unsigned int i = 0; i < repeat; ++i) {
    output->writer(output, chr);
  }
  return repeat;
}

int putchar_output(PrintOutput* output, int chr) {
  return putchar_n_output(output, chr, 1);
}

static int putstr_output(PrintOutput* output, const char* str) {
  int len = 0;
  while (*str) {
    len += putchar_output(output, *str++);
  }
  return len;
}

// Do not use this in here, just for external use
int putchar(int chr) {
  PrintOutput output = serial_output;
  return putchar_output(&output, chr);
}

static size_t uint_to_str(uint64_t num, char* out, char fmt) {
  size_t len = 0;
  unsigned base = fmt == 'u' ? 10 : 16;

  if (num) {
    char* start = out;

    while (num) {
      uint64_t digit = num % base;
      char hex_a_char = fmt == 'X' ? 65 : 97;
      char new_char = digit >= 10 ? hex_a_char : 48;
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
                                  va_list args, PrintOutput* output) {
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
      const char* str = va_arg(args, const char*);
      size_t str_len = strlen(str);
      if (str_len < padding_len) {
        len += putchar_n_output(output, ' ', padding_len - str_len);
      }
      len += putstr_output(output, str);
      fmt++;
      break;
    }
    case 'i': // Signed decimal
    {
      char int_str[17];
      int num = va_arg(args, int);
      if (num < 0) {
        len += putchar_output(output, '-');
        num = abs(num);
      }
      size_t int_len = uint_to_str(num, int_str, 'u');
      if (int_len < padding_len) {
        len += putchar_n_output(output, '0', padding_len - int_len);
      }
      len += putstr_output(output, int_str);
      fmt++;
      break;
    }
    case 'u': // Unsigned decimal
    case 'x': // unsigned hex
    case 'X': {
      // 64 bit hex plus null terminator
      char num_str[17];
      size_t num_len = uint_to_str(va_arg(args, size_t), num_str, *fmt);
      if (num_len < padding_len) {
        len += putchar_n_output(output, '0', padding_len - num_len);
      }
      len += putstr_output(output, num_str);
      fmt++;
      break;
    }
    case '%': // Escaped %
      len += putchar_output(output, *fmt++);
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
  PrintOutput output = serial_output;

  while (*fmt) {
    // Check for formatting arg
    if (*fmt == '%') {
      fmt++;
      args = handle_format_char(&len, &fmt, args, &output);
    } else {
      // Any non formatting character
      len += putchar_output(&output, *fmt++);
    }
  }

  return len;
}

int sprintf(char* str, const char* fmt, ...) {
  int len = 0;
  va_list args;
  va_start(args, fmt);
  PrintOutput output = {.writer = buffer_writer, .buf = str};

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      args = handle_format_char(&len, &fmt, args, &output);
    } else {
      len += putchar_output(&output, *fmt++);
    }
  }

  str[len] = '\0';
  va_end(args);
  return len;
}

void format_thread_name(char* out, int tid, const char* name) {
  // fill with spaces (skip last, null terminator done later)
  for (size_t idx = 0; idx < THREAD_NAME_MAX_LEN; ++idx) {
    out[idx] = ' ';
  }

  if (!name || !strlen(name)) {
    // If the thread had a stack issue
    if (tid == INVALID_THREAD) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_MAX_LEN - h_len;
      strncpy(&out[padding], hidden, h_len);
    } else {
      // Just show the ID number (assume max 999 threads)
      char idstr[4];
      int len = sprintf(idstr, "%u", tid);
      strcpy(&out[THREAD_NAME_MAX_LEN - len], idstr);
    }
  } else {
    size_t name_len = strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_MAX_LEN) {
      name_len = THREAD_NAME_MAX_LEN;
    }

    size_t padding = THREAD_NAME_MAX_LEN - name_len;
    strncpy(&out[padding], name, name_len);
  }

  out[THREAD_NAME_MAX_LEN] = '\0';
}

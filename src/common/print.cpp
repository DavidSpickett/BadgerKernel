#include "common/print.h"
#include "common/thread.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern "C" void __cxa_pure_virtual() {
  // TODO: exit error
  while (1) {
  }
}

class PrintOutput {
public:
  explicit PrintOutput(char* out) : m_out(out) {}

  virtual void write(int chr) const = 0;

  int putchar_n(int chr, unsigned int repeat) const {
    for (unsigned int i = 0; i < repeat; ++i) {
      write(chr);
    }
    return repeat;
  }

  int putchar(int chr) const {
    return putchar_n(chr, 1);
  }

  // Output a string to the given output stream
  // If num is SIZE_MAX, go until null terminator, otherwise num chars
  int putstr(const char* str, size_t num) const {
    int len = 0;
    while (*str && num) {
      len += putchar(*str++);
      if (num != SIZE_MAX) {
        --num;
      }
    }
    return len;
  }

protected:
  mutable char* m_out;
};

class SerialPrintOutput : public PrintOutput {
public:
  SerialPrintOutput() : PrintOutput(reinterpret_cast<char*>(UART_BASE)) {}

  void write(int chr) const final {
    volatile uint32_t* const UART0 = (uint32_t*)m_out;
    *UART0 = (uint32_t)chr;
    // We do not modify buf here, serial port doesn't move
  }
};

class BufferPrintOutput : public PrintOutput {
public:
  using PrintOutput::PrintOutput;

  void write(int chr) const final {
    // Writing to a memory buffer, so inc pointer
    *m_out++ = (char)chr;
  }
};

static const SerialPrintOutput serial_output;

// Do not use this in here, just for external use
int putchar(int chr) {
  return serial_output.putchar_n(chr, 1);
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

// Parse an unsigned integer from in, returning SIZE_MAX
// if it is not an integer. Otherwise return the number
// and update in to point to the char after it.
static size_t consume_uint(const char** in) {
  if (!isdigit((unsigned char)**in)) {
    return SIZE_MAX;
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

// Note that *fmt_chr here will point to the % not the char
// In case we don't recognise the format and need to print everything including
// the %
static va_list handle_format_char(int* out_len, const char** fmt_chr,
                                  va_list args, const PrintOutput& output) {
  // Save a bunch of * by making a copy now and assigning back at the end.
  // Also allows us to print the whole formatter if it's not recongised
  // +1 to consume the %
  const char* fmt = *fmt_chr + 1;
  int len = 0;

  size_t padding_len = SIZE_MAX;
  if (*fmt == '*') {
    int arg_pad = va_arg(args, int);
    padding_len = arg_pad > 0 ? arg_pad : 0;
    // consume the *
    ++fmt;
  } else {
    padding_len = consume_uint(&fmt);
    if (padding_len == SIZE_MAX) {
      padding_len = 0;
    }
  }

  // Check for precision
  size_t precision = SIZE_MAX;
  if (*fmt == '.') {
    // Consume the .
    ++fmt;
    if (*fmt == '*') {
      int precision_arg = va_arg(args, int);
      if (precision_arg >= 0) {
        precision = precision_arg;
      }
      // consume the *
      ++fmt;
    } else {
      precision = consume_uint(&fmt);
    }
  }

  switch (*fmt) {
    case 's': // string
    {
      const char* str = va_arg(args, const char*);
      if (!str) {
        str = "(null)";
      }

      size_t str_len = strlen(str);
      if (str_len > precision) {
        str_len = precision;
      }
      if (str_len < padding_len) {
        len += output.putchar_n(' ', padding_len - str_len);
      }
      len += output.putstr(str, precision);
      fmt++;
      break;
    }
    case 'i': // Signed decimal
    {
      char int_str[17];
      int num = va_arg(args, int);
      if (num < 0) {
        len += output.putchar('-');
        num = abs(num);
      }
      size_t int_len = uint_to_str(num, int_str, 'u');
      if (int_len < padding_len) {
        len += output.putchar_n('0', padding_len - int_len);
      }
      len += output.putstr(int_str, SIZE_MAX);
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
        len += output.putchar_n('0', padding_len - num_len);
      }
      len += output.putstr(num_str, SIZE_MAX);
      fmt++;
      break;
    }
    case '%': // Escaped %
      len += output.putchar(*fmt++);
      break;
    default: {
      // Consume unknown char
      ++fmt;
      // Echo back that char and any padding/precision
      size_t out_sz = fmt - *fmt_chr;
      len += output.putstr(*fmt_chr, out_sz);
      break;
    }
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
      args = handle_format_char(&len, &fmt, args, serial_output);
    } else {
      // Any non formatting character
      len += serial_output.putchar(*fmt++);
    }
  }

  return len;
}

int sprintf(char* str, const char* fmt, ...) {
  int len = 0;
  va_list args;
  va_start(args, fmt);
  BufferPrintOutput output(str);

  while (*fmt) {
    if (*fmt == '%') {
      args = handle_format_char(&len, &fmt, args, output);
    } else {
      len += output.putchar(*fmt++);
    }
  }

  str[len] = '\0';
  va_end(args);
  return len;
}

void format_thread_name(char* out, int tid, const char* name) {
  char thread_id[4];

  if (!name || !strlen(name)) {
    // If the thread had a stack issue
    if (tid == INVALID_THREAD) {
      name = "<HIDDEN>";
    } else {
      // Just show the ID number
      sprintf(thread_id, "%u", tid);
      name = thread_id;
    }
  }

  sprintf(out, "%*s", THREAD_NAME_MAX_LEN, name);
}

const char* text_colour(enum TextColour colour) {
  switch (colour) {
    case eReset:
      return "\033[0m";
    case eYellow:
      return "\033[0;33m";
    default:
      return "<unknown text colour>";
  }
}

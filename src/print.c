#include "print.h"

#ifdef linux

#include <stdio.h>

void print(const char* str) {
  printf("%s", str);
}

#else

void print(const char *s) {
  // This must be a an int write not a char
  volatile unsigned int * const UART0 = (unsigned int *)UART_BASE;
  while(*s) {
    *UART0 = (unsigned int)*s++;
  }
}

#endif

// unsigned base 10 only
size_t uint_to_str(uint32_t num, char* out) {
  size_t len = 0;

  if (!num) {
    *out = '0';
    len = 1;
  } else {
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
  }

  // Null terminate
  *(out+1) = '\0';

  return len;
}

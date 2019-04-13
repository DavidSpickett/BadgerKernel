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

void print(const char *s) {
  volatile unsigned int * const UART0 = (unsigned int *)0x09000000;
  while(*s != '\0') {
    *UART0 = (unsigned int)(*s++);
  }
}

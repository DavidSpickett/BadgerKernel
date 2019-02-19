void print(const char *s) {
  volatile unsigned int * const UART0DR = (unsigned int *)0x4000C000;
  while(*s != '\0') {
    *UART0DR = (unsigned int)(*s++);
  }
}

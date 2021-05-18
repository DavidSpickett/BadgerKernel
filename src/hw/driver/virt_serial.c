#include "common/serial_port.h"
#include <stdint.h>

#ifdef __aarch64__
#define UART_BASE 0x09000000
#elif defined __thumb__
#define UART_BASE 0x4000C000
#elif defined __arm__
#define UART_BASE 0x09000000
#else
#error No virt serial port address for this architecture!
#endif

static volatile uint32_t* const UART0 = (uint32_t*)UART_BASE;

static void virt_serial_init() {
  // In QEMU, we don't have to initialize UART
}

static void virt_serial_putchar(int c) {
  *UART0 = (uint32_t)c;
}

SerialPort serial_port = {
    .init = virt_serial_init,
    .putchar = virt_serial_putchar,
};

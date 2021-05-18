#ifndef COMMON_SERIAL_PORT
#define COMMON_SERIAL_PORT

typedef struct {
  void (*init)(void);
  void (*putchar)(int);
} SerialPort;

extern SerialPort serial_port;

#endif /* COMMON_SERIAL_PORT */

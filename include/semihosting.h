#ifndef SEMIHOSTING_H
#define SEMIHOSTING_H

/* Functions for interacting with the Qemu host machine */

__attribute__((noreturn)) void qemu_exit(void);
void qemu_print(const char* msg);

#endif /* ifdef SEMIHOSTING_H */

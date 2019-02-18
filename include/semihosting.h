#ifndef SEMIHOSTING_H
#define SEMIHOSTING_H

/* Functions for interacting with the Qemu host machine */

void qemu_exit();
void qemu_print(const char* msg);

#endif /* ifdef SEMIHOSTING_H */

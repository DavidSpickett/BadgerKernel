#ifndef SEMIHOSTING_H
#define SEMIHOSTING_H

#include <stddef.h>

// Semihosting operation codes
#define SYS_OPEN   0x01
#define SYS_CLOSE  0x02
#define SYS_WRITE  0x05
#define SYS_READ   0x06
#define SYS_SEEK   0x0A
#define SYS_FLEN   0x0C
#define SYS_REMOVE 0x0E
#define SYS_SYSTEM 0x12
#define SYS_EXIT   0x18

size_t generic_semihosting_call(size_t operation, size_t* parameters);

#endif /* ifdef SEMIHOSTING_H */

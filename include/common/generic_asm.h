#ifndef COMMON_GENERIC_ASM_H
#define COMMON_GENERIC_ASM_H

// Defines for writing inline assembly that works
// for Arm/Thumb/AArch64

#ifdef __aarch64__
#define RCHR             "x"
#define SEMIHOSTING_CALL "hlt 0xf000"
#define PTR_SIZE         "8"
#define BLR              "blr"
#define SYSCALL_REG      "20"

#elif defined __thumb__
#define RCHR             "r"
#define SEMIHOSTING_CALL "bkpt 0xab"
#define PTR_SIZE         "4"
#define BLR              "blx"
#define SYSCALL_REG      "8"

#else /* Arm */
#define RCHR             "r"
#define SEMIHOSTING_CALL "svc 0x123456"
#define PTR_SIZE         "4"
#define BLR              "blx"
#define SYSCALL_REG      "8"

#endif

#endif /* ifdef COMMON_GENERIC_ASM_H */

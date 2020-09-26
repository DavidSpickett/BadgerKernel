#ifndef PORT_ARM_COMMON_H
#define PORT_ARM_COMMON_H

#define YIELD_ASM \
  asm volatile("svc %0" : : "i"(svc_thread_switch) : "memory")

#endif /* ifdef PORT_ARM_COMMON_H */

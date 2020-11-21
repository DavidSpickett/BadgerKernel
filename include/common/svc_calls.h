#ifndef COMMON_SVC_CALLS_H
#define COMMON_SVC_CALLS_H

#ifdef __ASSEMBLER__
#define ENUM_START
#define ENUM_VALUE(key, value) .set key, value
#define ENUM_END(typename)
#else
#define ENUM_START             typedef enum {
#define ENUM_VALUE(key, value) key = value,
#define ENUM_END(typename)                                                     \
  }                                                                            \
  typename;
#endif

ENUM_START
ENUM_VALUE(svc_thread_switch, 0)
ENUM_VALUE(svc_enable_timer, 1)
ENUM_VALUE(svc_disable_timer, 2)
ENUM_VALUE(svc_syscall, 21)
ENUM_END(SVCCode)

#endif /* ifdef COMMON_SVC_CALLS_H */

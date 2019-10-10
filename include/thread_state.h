#ifndef THREAD_STATE_H
#define THREAD_STATE_H

#ifdef __ASSEMBLER__
#define ENUM_START
#define ENUM_VALUE(key, value) .set key, value
#define ENUM_END(typename)
#else
#define ENUM_START typedef enum{
#define ENUM_VALUE(key,value) key=value,
#define ENUM_END(typename)} typename;
#endif

ENUM_START
ENUM_VALUE(init,      0)
ENUM_VALUE(running,   1)
ENUM_VALUE(suspended, 2)
ENUM_VALUE(waiting,   3)
ENUM_VALUE(finished,  4)
ENUM_VALUE(cancelled, 5)
ENUM_END(ThreadState)

ENUM_START
ENUM_VALUE(svc_thread_switch, 0)
ENUM_VALUE(svc_semihosting,   1)
ENUM_VALUE(svc_enable_timer,  2)
ENUM_VALUE(svc_disable_timer, 3)
ENUM_END(SVCCodes)

#endif /* ifdef THREAD_STATE_H */

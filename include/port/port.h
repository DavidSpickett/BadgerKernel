#ifndef PORT_PORT_H
#define PORT_PORT_H

#ifdef __aarch64__
// TODO: later these headers will diverge
#include "arm.h"
#elif defined __thumb__
#include "arm.h"
#elif defined __arm__
#include "arm.h"
#else
#error Unknown architecture!
#endif

#endif /* ifdef PORT_PORT_H */

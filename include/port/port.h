#ifndef PORT_PORT_H
#define PORT_PORT_H

#ifdef __aarch64__
#include "aarch64.h"
#elif defined __thumb__
#include "thumb.h"
#elif defined __arm__
#include "arm.h"
#else
#error Unknown architecture!
#endif

#endif /* ifdef PORT_PORT_H */

#ifndef PRINT_H
#define PRINT_H

#ifdef linux
#include <stdio.h>
#include <inttypes.h>
#else
#include <stdint.h>
#include <string.h>

/* For some reason the bare metal toolchains
   don't define these macros. */
#if defined(__LP64__)
#define PRIX64 "lX"
#else
#define PRIX64 "llX"
#endif // ifdef __LP64__

int printf(const char* fmt, ...);
int sprintf(char* str, const char* fmt, ...);

#endif // !linux

#endif /* ifdef PRINT_H */

#ifndef PRINT_H
#define PRINT_H

#ifdef linux
#include <stdio.h>
#else
#include <stdint.h>
#include <string.h>

int printf(const char *fmt, ...);
int sprintf(char* str, const char* fmt, ...);

#endif // !linux

#endif /* ifdef PRINT_H */

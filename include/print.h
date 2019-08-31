#ifndef PRINT_H
#define PRINT_H

#ifdef linux
#include <stdio.h>
#else
#include <stdint.h>
#include <string.h>

void printf(const char *fmt, ...);

#endif // !linux

size_t uint_to_str(unsigned num, char* out);

#endif /* ifdef PRINT_H */

#ifndef PRINT_H
#define PRINT_H

#include <stdint.h>
#include <string.h>

void print(const char *fmt, ...);
size_t uint_to_str(unsigned num, char* out);

#endif /* ifdef PRINT_H */

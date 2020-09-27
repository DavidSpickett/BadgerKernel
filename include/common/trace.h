#ifndef COMMON_TRACE_H
#define COMMON_TRACE_H

#include "port/port.h"
#include <stddef.h>
#include <stdint.h>

// Only works properly for Arm
#if defined __arm__ && !defined __thumb__
typedef struct {
  const char* name;
  void* start;
  void* end;
} Symbol;

void print_backtrace(RegisterContext ctx, const Symbol* symbols,
                     size_t num_symbols);
#endif

#endif /* ifdef COMMON_TRACE_H */

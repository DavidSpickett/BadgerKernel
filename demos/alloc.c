#include "thread.h"
#include "util.h"
#include "alloc.h"
#include <stdint.h>

__attribute__((annotate("oclint:suppress[high ncss method]")))
void basic_types(void) {
  // Anything <=8 bytes
  uint8_t*  u08 = malloc(sizeof(uint8_t));
  uint16_t* u16 = malloc(sizeof(uint16_t));
  uint32_t* u32 = malloc(sizeof(uint32_t));
  uint64_t* u64 = malloc(sizeof(uint64_t));

  // Assign some known values
  *u08 = 0xFF;
  *u16 = 0xF0F0;
  *u32 = 0x0A0A0A0A;
  *u64 = 0xABCDABCDABCDABCD;

  // No one should overwrite another
  ASSERT(*u08 == 0xFF);
  ASSERT(*u16 == 0xF0F0);
  ASSERT(*u32 == 0x0A0A0A0A);
  ASSERT(*u64 == 0xABCDABCDABCDABCD);

  // TODO: printf %x for some log output

  // Free the second item
  free(u16);
  // Should be the same ptr as before
  uint16_t* new_u16 = malloc(sizeof(uint16_t));
  ASSERT(new_u16 == u16);
  free(new_u16);

  // Clean up the rest
  free(u08);
  free(u32);
  free(u64);
}

void large_alloc(void) {
  // Buffer >1 block size
  const size_t alloc_size = 123;
  uint8_t* temp = malloc(alloc_size);

  // canary which should *not* be allocated
  // 1 block after temp, since that uses multiple
  uint32_t* temp_check = malloc(sizeof(uint32_t));
  *temp_check = 0xdeadbeef;

  // Write to buffer, should not affect temp_check
  for (size_t i=0; i < alloc_size; ++i) {
    temp[i] = i;
  }

  ASSERT(*temp_check == 0xdeadbeef);
  free(temp);

  /* This should not be the same as temp, since it's
     bigger and there's an allocated guard right after
     where temp was. */
  uint8_t* new_temp = malloc(alloc_size*2);
  ASSERT(new_temp != temp);
  free(new_temp);

  /* This one *will* give us the same ptr.
     As it fits and has the same alignment as temp. */
  uint64_t* temp64 = malloc(sizeof(uint64_t));
  ASSERT((uint8_t*)temp64 == temp);
  free(temp64);

  free(temp_check);
}

void fragmented(void) {
  /* Shouldn't be able to allocate large objects
     in a fragmented heap. */

  size_t num_allocations = 64;
  uint64_t* allocated[num_allocations];
  for(size_t i=0; i < num_allocations; ++i) {
    // This will start to return NULL at some point
    // As long as we fill the heap that's fine
    allocated[i] = malloc(sizeof(uint64_t));
  }

  // free every second one to cause fragmentation
  for (size_t i=0; i < num_allocations; i+=2) {
    free(allocated[i]);
  }

  // small alloc is fine
  uint32_t* less_than_a_block = malloc(sizeof(uint32_t*));
  ASSERT(less_than_a_block != NULL);
  free(less_than_a_block);

  // >1 block fails
  ASSERT(malloc(64) == NULL);
}

void errors() {
  // Shouldn't be able to allocate > heap size
  ASSERT(malloc(2048) == NULL);
}

void setup(void)
{
  config.log_scheduler = false;

  add_named_thread(basic_types,    "basic_types");
  add_named_thread(large_alloc,    "large_alloc");
  add_named_thread(fragmented,     "fragmented");
  add_named_thread(errors,         "errors");
}

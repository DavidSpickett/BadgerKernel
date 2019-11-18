#include "alloc.h"
#include "thread.h"
#include "util.h"
#include <stdint.h>

void basic_types(void) {
  // Anything <=8 bytes
  uint8_t* u08 = malloc(sizeof(uint8_t));
  uint16_t* u16 = malloc(sizeof(uint16_t));
  uint32_t* u32 = malloc(sizeof(uint32_t));
  uint64_t* u64 = malloc(sizeof(uint64_t));

  // Assign some known values
  *u08 = 0xFF;
  *u16 = 0xF0F0;
  *u32 = 0x0A0A0A0A;
  *u64 = 0xABCDABCDABCDABCD;

  // No one should overwrite another
  printf(" u8: 0x%x\n", *u08);
  printf("u16: 0x%x\n", *u16);
  printf("u32: 0x%x\n", *u32);
  /* Rather than go through the hassle of getting
     64 bit types to print correctly. */
  assert(*u64 == 0xABCDABCDABCDABCD);

  // Free the second item
  free(u16);
  // Should be the same ptr as before
  uint16_t* new_u16 = malloc(sizeof(uint16_t));
  assert(new_u16 == u16);
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
  for (size_t i = 0; i < alloc_size; ++i) {
    temp[i] = i;
  }

  assert(*temp_check == 0xdeadbeef);
  free(temp);

  /* This should not be the same as temp, since it's
     bigger and there's an allocated guard right after
     where temp was. */
  uint8_t* new_temp = malloc(alloc_size * 2);
  assert(new_temp != temp);
  free(new_temp);

  /* This one *will* give us the same ptr.
     As it fits and has the same alignment as temp. */
  uint64_t* temp64 = malloc(sizeof(uint64_t));
  assert((uint8_t*)temp64 == temp);
  free(temp64);

  free(temp_check);
}

void fragmented(void) {
  /* Shouldn't be able to allocate large objects
     in a fragmented heap. */

  size_t num_allocations = 64;
  uint64_t* allocated[num_allocations];
  for (size_t i = 0; i < num_allocations; ++i) {
    // This will start to return NULL at some point
    // As long as we fill the heap that's fine
    allocated[i] = malloc(sizeof(uint64_t));
  }

  // free every second one to cause fragmentation
  for (size_t i = 0; i < num_allocations; i += 2) {
    free(allocated[i]);
  }

  // small alloc is fine
  uint32_t* less_than_a_block = malloc(sizeof(uint32_t*));
  assert(less_than_a_block != NULL);
  free(less_than_a_block);

  // >1 block fails
  void* greater_than_block = malloc(64);
  assert(!greater_than_block);

  // cleanup
  for (size_t i = 1; i < num_allocations; i += 2) {
    free(allocated[i]);
  }
}

void errors() {
  // Shouldn't be able to allocate > heap size
  void* bad_malloc = malloc(99999);
  assert(!bad_malloc);
}

void realloc_more() {
  // Realloc with nullptr is just malloc
  size_t array_sz = 8;
  uint32_t* foo = realloc(NULL, array_sz * sizeof(uint32_t));
  assert(foo);

  for (size_t i = 0; i < array_sz; ++i) {
    foo[i] = i;
  }

  // Realloc fails, the original is unharmed
  void* failed_realloc = realloc(foo, 123456);
  assert(!failed_realloc);

  for (size_t i = 0; i < array_sz; ++i) {
    assert(foo[i] == i);
  }

  size_t num_pad_allocs = 2;
  uint64_t pad_value = 0xcafef00ddeadbeef;
  uint64_t* pad_allocs[num_pad_allocs];
  for (size_t i = 0; i < num_pad_allocs; ++i) {
    pad_allocs[i] = malloc(sizeof(uint64_t));
    *pad_allocs[i] = pad_value;
  }

  /* Free the first to leave a gap for foo to grow */
  free(pad_allocs[0]);

  // Realloc to 2 blocks
  uint32_t* old_foo = foo;
  foo = realloc(foo, 16 * sizeof(uint32_t));
  assert(foo == old_foo);

  // foo's data was copied
  for (size_t i = 0; i < array_sz; ++i) {
    assert(foo[i] == i);
  }
  // Rest is uninitialised
  assert(foo[array_sz] != array_sz);

  // intact because we copy based on the old size
  assert(*pad_allocs[0] == pad_value);
  // last one is intact
  assert(*pad_allocs[1] == pad_value);

  // cleanup
  free(foo);
  free(pad_allocs[1]);
}

void realloc_less() {
  // Fill up some space that we'll realloc into later
  uint64_t* pad = malloc(sizeof(uint64_t));

  /* Canary to make sure we don't copy based on the
    original size only. */
  uint64_t canary_value = 0x1122334455667788;
  uint64_t* canary = malloc(sizeof(uint64_t));
  *canary = canary_value;

  /* Initially two blocks so we have
     pad canary foo[0] foo[1] */
  size_t array_sz = 64;
  uint8_t* foo = malloc(array_sz);
  for (size_t i = 0; i < array_sz; ++i) {
    foo[i] = i;
  }

  // Gives us space at the beginning for the new foo
  free(pad);

  size_t new_array_sz = 32;
  foo = realloc(foo, new_array_sz);
  // Now is where pad was, with the canary after it
  assert(foo == (uint8_t*)pad);

  // Canary is intact if we used the *new* size to copy
  assert(*canary == canary_value);

  free(canary);
  free(foo);
}

void realloc_free() {
  /* Check that when we realloc we do so based on what
     space we *could* have if the original allocation
     were freed. */
  uint32_t* allocation = malloc(sizeof(uint32_t));

  // Fill up the rest of the heap
  size_t num_padding = 64;
  uint32_t* pad_allocs[num_padding];
  for (size_t i = 0; i < num_padding; ++i) {
    pad_allocs[i] = malloc(sizeof(uint32_t));
  }

  // Free the one in front of the first allocation
  free(pad_allocs[0]);

  uint32_t* old_allocation = allocation;
  // Two blocks requires freeing first
  allocation = realloc(allocation, 64);
  assert(allocation);
  assert(old_allocation == allocation);

  free(allocation);
  for (size_t i = 1; i < num_padding; ++i) {
    free(pad_allocs[i]);
  }
}

void realloc_fail() {
  /* Make sure that we restore the tag after
     a failed realloc. */
  size_t num_allocs = 64;
  uint32_t* allocs[num_allocs];
  for (size_t i = 0; i < num_allocs; ++i) {
    allocs[i] = malloc(sizeof(uint32_t));
  }

  void* realloc_fails = realloc(allocs[0], 64);
  assert(!realloc_fails);
  /* If we forgot to restore the tag then
     this would be successful. */
  void* malloc_fails = malloc(sizeof(uint32_t));
  assert(!malloc_fails);

  for (size_t i = 0; i < num_allocs; ++i) {
    free(allocs[i]);
  }
}

void setup(void) {
  config.log_scheduler = false;

  add_named_thread(basic_types, "basic_types");
  add_named_thread(large_alloc, "large_alloc");
  add_named_thread(fragmented, "fragmented");
  add_named_thread(errors, "errors");
  add_named_thread(realloc_more, "realloc_more");
  add_named_thread(realloc_less, "realloc_less");
  add_named_thread(realloc_free, "realloc_free");
  add_named_thread(realloc_fail, "realloc_fail");
}

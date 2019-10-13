#include "thread.h"
#include "util.h"
#include "alloc.h"
#include <stdint.h>

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
  printf(" u8: 0x%x\n", *u08);
  printf("u16: 0x%x\n", *u16);
  printf("u32: 0x%x\n", *u32);
  /* Rather than go through the hassle of getting
     64 bit types to print correctly. */
  ASSERT(*u64 == 0xABCDABCDABCDABCD);

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

void free_clears_tags() {
  /* something larger than a block so that it covers
     a potential tag location. */
  size_t foo_sz = 64;
  uint8_t* foo = malloc(foo_sz);
  ASSERT(foo);

  //fill this with a number > blocks in the heap
  for (size_t i=0; i<foo_sz; ++i) {
    foo[i] = 0xFF;
  }

  /* Something after to check that free didn't
     overshoot clearing tag locations */
  int* canary = malloc(sizeof(int));

  free(foo);

  /* Can't ask whether a pointer is still allocated.
     Next best is to keep allocating and check it never
     gives us the address of the canary.
     Since freeing foo has also cleared some blocks. */
  size_t temps_sz = 5;
  int* temps[temps_sz];
  for (int i=0; i<temps_sz; ++i) {
    temps[i] = malloc(sizeof(int));
    /* If free didn't clear all potential tag locations
       it'd think we'd run out of space. */
    ASSERT(temps[i]);
    // If free free'd canary, it could give us its pointer
    ASSERT(temps[i] != canary);
  }
  for (int i=0; i<temps_sz; ++i) {
    free(temps[i]);
  }

  free(canary);
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

  // cleanup
  for (size_t i=1; i < num_allocations; i+=2) {
    free(allocated[i]);
  }
}

void errors() {
  // Shouldn't be able to allocate > heap size
  ASSERT(malloc(2048) == NULL);
}

void realloc_more() {
  // Realloc with nullptr is just malloc
  size_t array_sz = 25;
  uint8_t* foo = realloc(NULL, array_sz);
  ASSERT(foo);

  for(size_t i=0; i<array_sz; ++i) {
    foo[i] = i;
  }

  // Realloc fails, the origninal is unharmed
  ASSERT(!realloc(foo, 123456));

  for(size_t i=0; i<array_sz; ++i) {
    ASSERT(foo[i] == i);
  }

  size_t num_pad_allocs = 4;
  // Deliberatley 64 bit type here
  uint64_t pad_value = 0xcafef00ddeadbeef;
  uint64_t* pad_allocs[num_pad_allocs];
  for (size_t i=0; i<num_pad_allocs; ++i) {
    pad_allocs[i] = malloc(sizeof(uint64_t));
    *pad_allocs[i] = pad_value;
  }

  /* Free the first two to leave a gap for foo
     to be realloc-ed into. */
  free(pad_allocs[0]);
  free(pad_allocs[1]);
  free(pad_allocs[2]);

  // Realloc to 3 blocks
  uint8_t* old_foo = foo;
  foo = realloc(foo, 64);
  ASSERT(foo == (uint8_t*)pad_allocs[0]);

  // foo's data was copied
  for (size_t i=0; i<array_sz; ++i) {
    ASSERT(foo[i] == i);
  }
  // Rest is uninitialised
  ASSERT(foo[array_sz] != array_sz);

  // padding 0/1/2 were overwritten
  for (size_t i=0; i<3; ++i) {
    ASSERT(*pad_allocs[0] != pad_value);
  }
  // last one is intact
  ASSERT(*pad_allocs[3] == pad_value);

  // A new allocation goes where foo was originally
  uint64_t* temp = malloc(sizeof(uint64_t));
  ASSERT(temp == (uint64_t*)old_foo);
  free(temp);

  // cleanup
  free(foo);
  free(pad_allocs[3]);
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
     pad canary foo */
  size_t array_sz = 32;
  uint8_t* foo = malloc(array_sz);
  for (size_t i=0; i<array_sz; ++i) {
    foo[i] = i;
  }

  // Gives us space at the beginning for the new foo
  free(pad);

  size_t new_array_sz = 16;
  foo = realloc(foo, new_array_sz);
  // Now is where pad was, with the canary after it
  ASSERT(foo == (uint8_t*)pad);

  // Canary is intact if we used the *new* size to copy
  ASSERT(*canary == canary_value);

  free(canary);
  free(foo);
}

void realloc_free() {
  // TODO: make this test work
  /* Check that when we realloc we do so based on what
     space we *could* have if the original allocation
     were freed. */
  uint32_t* allocation = malloc(sizeof(uint32_t));

  // Fill up the heap
  size_t num_padding = 64;
  uint32_t* pad_allocs[num_padding];
  for (size_t i=0; i<num_padding; ++i) {
    pad_allocs[i] = malloc(sizeof(uint32_t));
  }

  // Free the one in front of the first allocation
  free(pad_allocs[0]);

  //uint32_t* old_allocation = allocation;
  allocation = realloc(allocation, 32);
  ASSERT(!allocation);
  // ASSERT(allocation);
  // ASSERT(old_allocation == allocation);

  free(allocation);
  for (size_t i=1; i<num_padding; ++i) {
    free(pad_allocs[i]);
  }
}

void setup(void)
{
  config.log_scheduler = false;

  add_named_thread(basic_types,      "basic_types");
  add_named_thread(large_alloc,      "large_alloc");
  add_named_thread(fragmented,       "fragmented");
  add_named_thread(errors,           "errors");
  add_named_thread(free_clears_tags, "free_clears_tags");
  add_named_thread(realloc_more,     "realloc_more");
  add_named_thread(realloc_less,     "realloc_less");
  add_named_thread(realloc_free,     "realloc_free");
}

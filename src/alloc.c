#include "alloc.h"
#include "util.h"
#include <stdint.h>

#define BLOCK_SIZE    32
#define HEAP_SIZE     1024

uint8_t heap[HEAP_SIZE] __attribute__((aligned(BLOCK_SIZE)));
const uint8_t* heap_end = heap+HEAP_SIZE+1;

bool can_allocate(uint8_t* from, uint8_t num_blocks) {
  // Need a long enough run of free blocks
  // Start at 1 because we know the current block is free
  for (uint8_t i=1; i < num_blocks; from+=BLOCK_SIZE, ++i) {
    if ((from >= heap_end) || (*from != 0)) {
      return false;
    }
  }

  return true;
}

//TODO: mutex!!
void* malloc(size_t size) {
  uint8_t* search_ptr = heap;

  size_t align_amount = size >= 8 ? 8 : 4;
  size_t space_in_first_block = BLOCK_SIZE-align_amount;

  while (search_ptr < heap_end) {
    /* Each tag can describe a "run" of blocks
       used for a particualr object. */
    uint8_t allocated_blocks = *search_ptr;

    if (allocated_blocks == 0) {
      // Smallest allocation is one block
      uint8_t num_blocks = 1;

      if (size > space_in_first_block) {
        size_t remaining_bytes = size - space_in_first_block;
        /* Note that subsequent blocks can use all space
           since there's no tag/alignment. */
        // Round up here
        num_blocks += (remaining_bytes + BLOCK_SIZE) / BLOCK_SIZE;
      }

      if (can_allocate(search_ptr, num_blocks)) {
        // Allocate the block(s)
        *search_ptr = num_blocks;
        // return aligned pointer for use
        return search_ptr + align_amount;
      }

      // If the object doesn't fit we can skip this section
      allocated_blocks = num_blocks;
    }

    /* Skip forward by the number of allocated blocks.
       Or if we needed more than one but didn't find a
       free run from this point, the number of blocks
       the object would require. */
    search_ptr += BLOCK_SIZE*allocated_blocks;
  }

  return NULL; //!OCLINT
}

void free(void* ptr) {
  if (!ptr || (ptr >= (void*)heap_end)) {
    return;
  }

  // Round ptr down to nearest block
  size_t raw_ptr = (size_t)ptr;
  uint8_t* tag = (uint8_t*)( raw_ptr - (raw_ptr % BLOCK_SIZE));
  // Then clear the tag
  *tag = 0;
}

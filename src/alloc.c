#include "alloc.h"
#include "util.h"
#include <stdint.h>

#define BLOCK_SIZE    32
#define HEAP_SIZE     1024

uint8_t heap[HEAP_SIZE] __attribute__((aligned(BLOCK_SIZE)));
const uint8_t* heap_end = heap+HEAP_SIZE+1;

uint8_t* can_allocate(uint8_t* from, uint8_t num_blocks) {
  // Need a long enough run of free blocks
  // If we can't allocate, return the place we searched until
  // If we did return the original pointer
  uint8_t* search_ptr = from;
  for (uint8_t i=0; i < num_blocks; search_ptr+=BLOCK_SIZE, ++i) {
    if ((search_ptr >= heap_end) || (*search_ptr != 0)) {
      /* Skip ahead to the end of the run */
      return search_ptr + ((*search_ptr) * BLOCK_SIZE);
    }
  }

  return from;
}

//TODO: mutex!!
void* malloc(size_t size) {
  uint8_t* search_ptr = heap;

  size_t align_amount = size >= 8 ? 8 : 4;
  size_t space_in_first_block = BLOCK_SIZE-align_amount;

  // Smallest allocation is one block
  uint8_t num_blocks = 1;

  if (size > space_in_first_block) {
    size_t remaining_bytes = size - space_in_first_block;
    /* Note that subsequent blocks can use all space
       since there's no tag/alignment. */
    // Round up here
    num_blocks += (remaining_bytes + BLOCK_SIZE) / BLOCK_SIZE;
  }

  while (search_ptr < heap_end) {
    uint8_t* potential_alloc = can_allocate(search_ptr, num_blocks);
    if (potential_alloc == search_ptr) {
      // Allocate the blocks
      *search_ptr = num_blocks;
      // Return aligned ptr for use
      return search_ptr + align_amount;
    }

    // Skip forward and carry on searching
    search_ptr = potential_alloc;
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
  uint8_t allocated_blocks = *tag;

  // Clear all possible tag locations in the run of blocks
  for ( ; allocated_blocks > 0; --allocated_blocks, tag+=BLOCK_SIZE) {
    *tag = 0;
  }
}

#include "alloc.h"
#include "util.h"
#include <stdint.h>

#define BLOCK_SIZE    32
#define HEAP_SIZE     1024

uint8_t heap[HEAP_SIZE] __attribute__((aligned(BLOCK_SIZE)));
const size_t* heap_end = (size_t*)(heap+HEAP_SIZE);

size_t* can_allocate(size_t* from, size_t num_blocks) {
  // Need a long enough run of free blocks
  // If we can't allocate, return the place we searched until
  // If we did return the original pointer
  size_t* search_ptr = from;
  size_t stride = BLOCK_SIZE / sizeof(size_t);
  for (size_t i=0; i < num_blocks; search_ptr+=stride, ++i) {
    if ((search_ptr >= heap_end) || (*search_ptr != 0)) {
      /* Skip ahead to the end of the run */
      return search_ptr + ((*search_ptr) * stride);
    }
  }

  return from;
}

//TODO: mutex!!
void* malloc(size_t size) {
  size_t* search_ptr = (size_t*)heap;

  size_t align_amount = sizeof(size_t);
  if (size >= 8) {
    align_amount = 8;
  }
  size_t space_in_first_block = BLOCK_SIZE-align_amount;

  // Smallest allocation is one block
  size_t num_blocks = 1;

  if (size > space_in_first_block) {
    size_t remaining_bytes = size - space_in_first_block;
    /* Note that subsequent blocks can use all space
       since there's no tag/alignment. */
    // Round up here
    num_blocks += (remaining_bytes + BLOCK_SIZE) / BLOCK_SIZE;
  }

  while (search_ptr < heap_end) {
    size_t* potential_alloc = can_allocate(search_ptr, num_blocks);
    if (potential_alloc == search_ptr) {
      // Allocate the blocks
      *search_ptr = num_blocks;
      // Return aligned ptr for use
      return ((uint8_t*)search_ptr) + align_amount;
    }

    // Skip forward and carry on searching
    search_ptr = potential_alloc;
  }

  return NULL; //!OCLINT
}

size_t* block_align_ptr(void* ptr) {
  size_t raw_ptr = (size_t)ptr;
  return (size_t*)( raw_ptr - (raw_ptr % BLOCK_SIZE));
}

void* realloc (void* ptr, size_t size) {
  void* new_ptr = malloc(size);

  // Acts like malloc
  if (!ptr) {
    return new_ptr;
  }

  // TODO: see whether we can realloc if we free first
  if (new_ptr) {
    size_t old_size = (*(block_align_ptr(ptr))) * BLOCK_SIZE;
    /* Since we only store in number of blocks, we will
       overestimate. So remove the alignment amount that'll
       be at the start of the first block.
       We can safely copy any overflow on the end though. */
    old_size -= ((size_t)ptr) % BLOCK_SIZE;

    // Use new size if we're shrinking the allocation
    size_t copy_size = size < old_size ? size : old_size;

    // Copy data to new location
    memcpy(new_ptr, ptr, copy_size);
    // Delete the original
    free(ptr);
  }

  return new_ptr;
}

void free(void* ptr) {
  if (!ptr || (ptr >= (void*)heap_end)) {
    return;
  }

  // Round ptr down to nearest block
  size_t* tag = block_align_ptr(ptr);
  size_t allocated_blocks = *tag;

  // Clear all possible tag locations in the run of blocks
  size_t stride = BLOCK_SIZE/sizeof(size_t);
  for ( ; allocated_blocks > 0; --allocated_blocks, tag+=stride) {
    *tag = 0;
  }
}

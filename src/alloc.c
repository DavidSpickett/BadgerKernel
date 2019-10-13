#include "alloc.h"
#include "util.h"
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE    32
#define HEAP_SIZE     1024
#define NUM_BLOCKS    HEAP_SIZE/BLOCK_SIZE

size_t block_tags[NUM_BLOCKS];
uint8_t heap[HEAP_SIZE] __attribute__((aligned(BLOCK_SIZE)));
const uint8_t* heap_end = heap+HEAP_SIZE;

void* find_free_space(size_t num_blocks) {
  size_t free_found = 0;
  size_t start_idx = 0;
  for (size_t idx = 0; idx<NUM_BLOCKS; ++idx) {
    if (block_tags[idx]) {
      // Reset run of free space
      free_found = 0;
      // Skip over blocks we know are allocated
      // (-1 / +1 here because of the for loop)
      idx += (block_tags[idx]-1);
      // Start new potential run of space
      start_idx = idx+1;
    } else {
      free_found++;

      if (free_found == num_blocks) {
        // Return pointer into actual heap space
        return heap+(start_idx*BLOCK_SIZE);
      }
    }
  }

  return NULL;
}

size_t pointer_to_tag_idx(void* ptr) {
  size_t raw_ptr = (size_t)ptr;
  raw_ptr -= (size_t)heap;
  return raw_ptr / BLOCK_SIZE;
}

//TODO: mutex!!
void* malloc(size_t size) {
  // Divide rounding up
  size_t num_blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
  void* alloc = find_free_space(num_blocks);

  if (alloc) {
    // Actually claim the space
    block_tags[pointer_to_tag_idx(alloc)] = num_blocks;
  }

  return alloc;
}

size_t* block_align_ptr(void* ptr) {
  size_t raw_ptr = (size_t)ptr;
  return (size_t*)( raw_ptr - (raw_ptr % BLOCK_SIZE));
}

__attribute__((annotate("oclint:suppress[prefer early exits and continue]")))
void* realloc (void* ptr, size_t size) {
  void* new_ptr = malloc(size);
  // realloc NULL is just malloc
  if (!ptr) {
    return new_ptr;
  }

  if (new_ptr) {
    size_t old_tag = block_tags[pointer_to_tag_idx(ptr)];
    // This will overshoot some but that's fine
    size_t old_size = old_tag * BLOCK_SIZE;
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

  block_tags[pointer_to_tag_idx(ptr)] = 0;
}

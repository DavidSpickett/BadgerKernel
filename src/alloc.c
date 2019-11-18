#include "alloc.h"
#include "util.h"
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 32
#define HEAP_SIZE  2048
#define NUM_BLOCKS HEAP_SIZE / BLOCK_SIZE

size_t block_tags[NUM_BLOCKS];
uint8_t heap[HEAP_SIZE] __attribute__((aligned(BLOCK_SIZE)));
const uint8_t* heap_end = heap + HEAP_SIZE;

void* find_free_space(size_t num_blocks) {
  size_t free_found = 0;
  size_t start_idx = 0;
  for (size_t idx = 0; idx < NUM_BLOCKS; ++idx) {
    if (block_tags[idx]) {
      // Reset run of free space
      free_found = 0;
      // Skip over blocks we know are allocated
      // (-1 / +1 here because of the for loop)
      idx += (block_tags[idx] - 1);
      // Start new potential run of space
      start_idx = idx + 1;
    } else {
      free_found++;

      if (free_found == num_blocks) {
        // Return pointer into actual heap space
        return heap + (start_idx * BLOCK_SIZE);
      }
    }
  }

  return NULL; //!OCLINT
}

size_t pointer_to_tag_idx(void* ptr) {
  size_t raw_ptr = (size_t)ptr;
  raw_ptr -= (size_t)heap;
  return raw_ptr / BLOCK_SIZE;
}

size_t to_blocks(size_t size) {
  return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

// TODO: mutex!!
void* malloc(size_t size) {
  // Divide rounding up
  size_t num_blocks = to_blocks(size);
  void* alloc = find_free_space(num_blocks);

  if (alloc) {
    // Actually claim the space
    block_tags[pointer_to_tag_idx(alloc)] = num_blocks;
  }

  return alloc;
}

void* realloc(void* ptr, size_t size) {
  // realloc NULL is just malloc
  if (!ptr) {
    return malloc(size);
  }

  size_t num_blocks = to_blocks(size);
  size_t tag_idx = pointer_to_tag_idx(ptr);
  size_t old_tag = block_tags[tag_idx];

  // Temporarily free the current allocation
  block_tags[tag_idx] = 0;
  // Look for space as if the current one didn't exist
  void* new_ptr = find_free_space(num_blocks);

  if (new_ptr) {
    // This will overshoot some but that's fine
    size_t old_size = old_tag * BLOCK_SIZE;
    // Use new size if we're shrinking the allocation
    size_t copy_size = size < old_size ? size : old_size;
    // Copy data to new location
    memcpy(new_ptr, ptr, copy_size);
    // Set new allocation tag
    block_tags[pointer_to_tag_idx(new_ptr)] = num_blocks;
  } else {
    // Restore original allocation
    block_tags[tag_idx] = old_tag;
  }

  return new_ptr;
}

void free(void* ptr) {
  if (!ptr || (ptr >= (void*)heap_end)) {
    return;
  }

  block_tags[pointer_to_tag_idx(ptr)] = 0;
}

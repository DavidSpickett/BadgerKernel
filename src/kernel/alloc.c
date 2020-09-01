#include "kernel/alloc.h"
#include "kernel/thread.h"
#include <stdint.h>
#include <string.h>

#define BLOCK_SIZE 32
#define HEAP_SIZE  2048
#define NUM_BLOCKS HEAP_SIZE / BLOCK_SIZE

typedef struct {
  int tid;
  size_t tag;
} BlockTag;
BlockTag block_tags[NUM_BLOCKS];
uint8_t heap[HEAP_SIZE] __attribute__((aligned(BLOCK_SIZE)));
const uint8_t* heap_end = heap + HEAP_SIZE;

// Tag walker
static size_t next_tag(size_t idx) {
  if (block_tags[idx].tag) {
    // Skip runs of allocated blocks
    idx += block_tags[idx].tag;
  } else {
    // One at a time over free blocks
    idx += 1;
  }
  // SIZE_MAX means walk ended
  return idx < NUM_BLOCKS ? idx : SIZE_MAX;
}

static size_t find_free_space(size_t num_blocks) {
  size_t free_found = 0;
  size_t start_idx = 0;
  size_t current_idx = 0;

  do {
    if (block_tags[current_idx].tag) {
      // Reset run of free space
      free_found = 0;
      // Start new potential run of space
      current_idx = next_tag(current_idx);
      start_idx = current_idx;
    } else {
      free_found++;
      current_idx = next_tag(current_idx);

      if (free_found == num_blocks) {
        return start_idx;
      }
    }
  } while (current_idx != SIZE_MAX);

  return SIZE_MAX;
}

static size_t pointer_to_tag_idx(void* ptr) {
  size_t raw_ptr = (size_t)ptr;
  raw_ptr -= (size_t)heap;
  return raw_ptr / BLOCK_SIZE;
}

static size_t to_blocks(size_t size) {
  return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

static void* to_heap_ptr(size_t tag_idx) {
  return heap + (tag_idx * BLOCK_SIZE);
}

void* k_malloc(size_t size) {
  if (k_has_no_permission(TPERM_ALLOC)) {
    return NULL;
  }

  // Divide rounding up
  size_t num_blocks = to_blocks(size);
  size_t alloc_idx = find_free_space(num_blocks);

  if (alloc_idx != SIZE_MAX) {
    // Actually claim the space
    block_tags[alloc_idx].tag = num_blocks;
    block_tags[alloc_idx].tid = k_get_thread_id();
    return to_heap_ptr(alloc_idx);
  }

  return NULL;
}

static bool can_realloc_free(void* ptr) {
  size_t tag_idx = pointer_to_tag_idx(ptr);
  if (
      // Can't free a NULL ptr
      !ptr ||
      // Can't free something outside the heap
      (ptr >= (void*)heap_end) ||
      // Can't free another thread's memory
      (block_tags[tag_idx].tid != k_get_thread_id())) {
    return false;
  }
  return true;
}

void* k_realloc(void* ptr, size_t size) {
  if (k_has_no_permission(TPERM_ALLOC)) {
    return NULL;
  }

  if (!ptr) {
    // realloc NULL is just malloc
    return k_malloc(size);
  }

  if (!can_realloc_free(ptr)) {
    return ptr;
  }

  size_t num_blocks = to_blocks(size);
  size_t tag_idx = pointer_to_tag_idx(ptr);
  size_t old_tag = block_tags[tag_idx].tag;

  // Temporarily free the current allocation
  block_tags[tag_idx].tag = 0;
  // Look for space as if the current one didn't exist
  size_t alloc_idx = find_free_space(num_blocks);

  if (alloc_idx != SIZE_MAX) {
    size_t old_size = old_tag * BLOCK_SIZE;
    // Use new size if we're shrinking the allocation
    size_t copy_size = size < old_size ? size : old_size;

    void* new_ptr = to_heap_ptr(alloc_idx);
    // Copy data to new location
    memcpy(new_ptr, ptr, copy_size);
    // Set new allocation tag
    block_tags[alloc_idx].tag = num_blocks;
    block_tags[alloc_idx].tid = k_get_thread_id();

    // Return the new address
    return new_ptr;
  }

  // Restore original allocation
  block_tags[tag_idx].tag = old_tag;
  // Return nullptr on failure
  return NULL;
}

void k_free_all(int tid) {
  size_t current_idx = 0;
  do {
    if (block_tags[current_idx].tag && block_tags[current_idx].tid == tid) {
      block_tags[current_idx].tag = 0;
    }
    current_idx = next_tag(current_idx);
  } while (current_idx != SIZE_MAX);
}

void k_free(void* ptr) {
  if (k_has_no_permission(TPERM_ALLOC) || !can_realloc_free(ptr)) {
    return;
  }

  size_t tag_idx = pointer_to_tag_idx(ptr);
  block_tags[tag_idx].tag = 0;
  block_tags[tag_idx].tid = 0;
}

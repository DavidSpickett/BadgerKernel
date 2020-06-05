#include "print.h"
#include "thread.h"
#include "util.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct __emutls_control {
  size_t size;
  // TODO: actually use this
  size_t alignment;
  union {
    uintptr_t index;
    void* address;
  } object;
  void* value;
} __emutls_control;

#define ALLOC_SPACE_SIZE 256
#define MAX_TLS_VARS     3
static void* address_arrays[MAX_THREADS][MAX_TLS_VARS];
static uint8_t alloc_space[ALLOC_SPACE_SIZE];
static uint8_t* alloc_ptr = alloc_space;
static uintptr_t emutls_var_count;

static void emutls_init_var(__emutls_control* control) {
  uint8_t* new_ptr = alloc_ptr + control->size;
  if (new_ptr > &alloc_space[ALLOC_SPACE_SIZE]) {
    printf("Ran out of TLS alloc space!\n");
    exit(1);
  } else {
    // Initialise the new value
    memcpy(alloc_ptr, control->value, control->size);

    // Put its address in this thread's address array
    address_arrays[k_get_thread_id()][control->object.index - 1] = alloc_ptr;

    // update for next allocation
    alloc_ptr = new_ptr;
  }
}

static uintptr_t emutls_get_index(__emutls_control* control) {
  // If it's the first time any thread has accessed this var...
  if (control->object.index == 0) {
    // Assign it an index (1+ because 0 is the invalid index)
    control->object.index = 1 + emutls_var_count++;
    if (emutls_var_count > MAX_TLS_VARS) {
      printf("Too many TLS vars!\n");
      exit(1);
    }
  }

  return control->object.index;
}

static void* emutls_get_var_addr(__emutls_control* control) {
  uintptr_t index = emutls_get_index(control);

  void* var_ptr = address_arrays[k_get_thread_id()][index - 1];
  // First time accessing the var on this thread
  if (var_ptr == NULL) {
    emutls_init_var(control);
  }

  return address_arrays[k_get_thread_id()][index - 1];
}

__attribute__((used)) // For LTO builds
void* __emutls_get_address(__emutls_control* control) {
  // TODO: all of this is *not* interrupt safe!
  // TODO: it might actually be after syscall transition
  return emutls_get_var_addr(control);
}

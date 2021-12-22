#include "common/assert.h"
#include "user/thread.h"

const char* globalmsg = "Using a global!";
// To prove we init .bss correctly
int zero_init_global = 0;

void other_worker() {
  log_event(globalmsg);
}

// The entry point
void worker(int arg1, int arg2, int arg3, int arg4) {
  // Check that loader passed on arguments
  assert(arg1 == 1);
  assert(arg2 == 2);
  assert(arg3 == 3);
  assert(arg4 == 4);

  // Check that loader passed on permissions.
  // The original caller didn't have alloc and it asked
  // the loader to remove kconfig for the new thread.
  uint16_t perm = permissions(0);
  assert(!(perm & TPERM_ALLOC));
  assert(!(perm & TPERM_KCONFIG));

  assert(zero_init_global == 0);

  // Parent set before the loader ran should be preserved
  // across the restart.
  int this_thread_tid = get_thread_id();
  int parent = INVALID_THREAD;
  bool got_parent = get_parent(CURRENT_THREAD, &parent);
  assert(got_parent);

  if (this_thread_tid == 1) {
    // First load has no parent
    assert(parent == INVALID_THREAD);
  } else if (this_thread_tid == 0) {
    // Second load's parent is load_again
    assert(parent == 2);
  } else {
    assert(0 && "Binary loaded as unexpected thread ID!");
  }

  /* Note that this *will* get reloaded when loadbinary
     loads the file again. So each time we'll add a new thread. */
  static bool add_new_thread = true;
  const char* msg = "Hello";
  log_event(msg);
  if (add_new_thread) {
    int tid = add_named_thread(other_worker, "other");
    log_event("Added new thread with ID %i!", tid);
    add_new_thread = false;
  }
  log_event("Goodbye");
}

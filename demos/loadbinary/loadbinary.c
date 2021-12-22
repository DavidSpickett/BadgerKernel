#include "common/assert.h"
#include "user/errno.h"
#include "user/thread.h"

void load_again() {
  const char* filename = "binary";

  // Fails because other_worker from binary is still active
  // (worker has finished though)
  errno = 0;
  assert(add_thread_from_file(filename) == INVALID_THREAD);
  assert(errno == E_NO_PAGE);
  yield();

  log_event("loading binary again");
  // Now that all threads from binary have finished
  // we can load it into the code page again
  int added = add_thread_from_file(filename);
  assert(added != INVALID_THREAD);

  // Cancelling it should release the page and let us load again
  assert(thread_cancel(added));
  log_event("binary thread cancelled");

  // Since this load will succeed it needs the expected arguments
  // and permissions.
  // Also check here that parent is preserved by restart
  ThreadArgs args = make_args(1, 2, 3, 4);
  ThreadFlags flags = {.is_file = true, .remove_permissions = TPERM_KCONFIG};
  int tid = add_thread(filename, &args, (void*)filename, &flags);
  assert(tid != INVALID_THREAD);
  set_child(tid);
  log_event("binary loaded again as thread ID %i", tid);
}

void setup(void) {
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);

  // Set permissions to something not the default to check
  // that the loader carries them forward.
  permissions(TPERM_ALLOC);

  // Declared like this so it's always on this thread's stack
  char filename[] = {'b', 'i', 'n', 'a', 'r', 'y', '\0'};
  ThreadArgs args = make_args(1, 2, 3, 4);
  // Here remove kernel config permission, meaning the new thread
  // will not have alloc or kconfig.
  ThreadFlags flags = {.is_file = true, .remove_permissions = TPERM_KCONFIG};
  assert(add_thread(filename, &args, (void*)filename, &flags) !=
         INVALID_THREAD);

  add_named_thread(load_again, "load_again");
  // Fails because single code page is in use
  assert(add_thread_from_file(filename) == INVALID_THREAD);

  // Reset path and arguments to make sure the loader doesn't rely on
  // copies from our stack. (the loader will only run once this thread exits)
  filename[1] = '\0';
  args.a1 = 0;
  args.a2 = 0;
  args.a3 = 0;
  args.a4 = 0;
}

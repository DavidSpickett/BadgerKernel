#include "common/print.h"
#include "user/thread.h"

// TODO: move somewhere general?
static const char* thread_state_to_str(ThreadState state) {
  switch (state) {
    case (init):
      return "init";
    case (running):
      return "running";
    case (suspended):
      return "suspended";
    case (waiting):
      return "waiting";
    case (finished):
      return "finished";
    case (cancelled):
      return "cancelled";
    default:
      return "unknown";
  }
}

void worker(int argc, char* argv[]) {
  (void)argv;
  if (argc > 1) {
    printf("ps expects no arguments");
    return;
  }

  for (int tid = 0;; ++tid) {
    const char* name;
    ThreadState state;
    // Already checked if valid thread
    bool valid = thread_name(tid, &name);

    // Must have hit max threads
    if (!valid) {
      break;
    }

    // Valid already checked above
    get_thread_state(tid, &state);
    const char* state_name = thread_state_to_str(state);

    int child_tid;
    get_child(tid, &child_tid);
    const char* child_name = NULL;
    if (child_tid != INVALID_THREAD) {
      thread_name(child_tid, &child_name);
    }

    printf("|-----------|\n");
    printf("| Thread %u\n", tid);
    printf("|-----------|\n");
    if (name) {
      printf("| Name      | %s\n", name);
    }
    printf("| State     | %s (%u)\n", state_name, state);
    if (child_name) {
      printf("| Child     | %s (%u)\n", child_name, child_tid);
    }
    printf("|-----------|\n");
  }
}

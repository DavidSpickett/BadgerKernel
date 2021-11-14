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
    printf("ps expects no arguments\n");
    return;
  }

  for (int tid = 0;; ++tid) {
    ThreadState state;
    char name[THREAD_NAME_SIZE];
    bool valid = thread_name(tid, name);
    // Must have hit max threads
    if (!valid) {
      break;
    }

    // Valid already checked above
    get_thread_state(tid, &state);
    const char* state_name = thread_state_to_str(state);

    int child_tid = INVALID_THREAD;
    get_child(tid, &child_tid);
    char child_name[THREAD_NAME_SIZE];
    if (child_tid != INVALID_THREAD) {
      thread_name(child_tid, child_name);
    }

    int parent_tid = INVALID_THREAD;
    get_parent(tid, &parent_tid);
    char parent_name[THREAD_NAME_SIZE];
    if (parent_tid != INVALID_THREAD) {
      thread_name(parent_tid, parent_name);
    }

    printf("|-----------|\n");
    printf("| Thread %u\n", tid);
    printf("|-----------|\n");

    if (strlen(name)) {
      printf("| Name      | %s\n", name);
    }

    printf("| State     | %s (%u)\n", state_name, state);

    if (parent_tid != INVALID_THREAD) {
      printf("| Parent    | ");
      if (strlen(parent_name)) {
        printf("%s (%i)\n", parent_name, parent_tid);
      } else {
        printf("%i\n", parent_tid);
      }
    }

    if (child_tid != INVALID_THREAD) {
      printf("| Child     | ");
      if (strlen(child_name)) {
        printf("%s (%i)\n", child_name, child_tid);
      } else {
        printf("%i\n", child_tid);
      }
    }

    printf("|-----------|\n");
  }
}

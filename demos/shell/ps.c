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

static void print_related_thread(int this_tid, const char* kind,
                                 bool(getter)(int, int*)) {
  int other_tid = INVALID_THREAD;
  bool got = getter(this_tid, &other_tid);
  if (!got) {
    return;
  }

  char other_name[THREAD_NAME_SIZE];
  if (other_tid != INVALID_THREAD) {
    thread_name(other_tid, other_name);
  }

  if (other_tid != INVALID_THREAD) {
    printf("%s", kind);
    if (strlen(other_name)) {
      printf("%s (%i)\n", other_name, other_tid);
    } else {
      printf("%i\n", other_tid);
    }
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

    if (strlen(name)) {
      printf("| %s (%u)\n", name, tid);
    } else {
      printf("| Thread %u\n", tid);
    }

    // Valid already checked above
    get_thread_state(tid, &state);
    const char* state_name = thread_state_to_str(state);
    printf("   State | %s (%u)\n", state_name, state);

    print_related_thread(tid, "  Parent | ", get_parent);
    print_related_thread(tid, "  Child  | ", get_child);
  }
}

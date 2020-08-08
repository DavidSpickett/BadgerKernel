#include "print.h"
#include "user/thread.h"

// TODO: move somewhere general?
static const char* thread_state_to_str(ThreadState state) {
  switch (state) {
    case(init):
      return "init";
    case(running):
      return "running";
    case(suspended):
      return "suspended";
    case(waiting):
      return "waiting";
    case(finished):
      return "finished";
    case(cancelled):
      return "cancelled";
    default:
      return "unknown";
  }
}

void worker(int argc, char* argv[]) {
  if (argc > 1) {
    printf("ps expects no arguments");
    return;
  }

  for (int tid=0; ; ++tid) {
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

    printf("|-----------|\n");
    printf("| Thread %u\n", tid);
    printf("|-----------|\n");
    printf("| Name      | %s\n", name);
    printf("| State     | %s (%u)\n", state_name, state);
    printf("|-----------|\n");
  }
}

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "thread.h"
#include "print.h"

#define THREAD_STACK_SIZE 512
#define THREAD_MSG_QUEUE_SIZE 5

struct Message {
  int src;
  int content;
};

struct Thread {
  uint8_t* stack_ptr;
  void (*current_pc)(void);
  void (*work)(void);
  uint8_t stack[THREAD_STACK_SIZE];
  struct Message messages[THREAD_MSG_QUEUE_SIZE];
  struct Message* next_msg;
  struct Message* end_msgs;
  bool msgs_full;
  int id;
};

extern void platform_yield(void**, void*);

static struct Thread scheduler_thread;
static struct Thread* current_thread;
struct Thread all_threads[MAX_THREADS];

bool is_valid_thread(int tid) {
  return (tid >= 0) &&
    (tid < MAX_THREADS) &&
    all_threads[tid].id != -1;
}

int get_thread_id() {
  return current_thread->id;
}

static void inc_msg_pointer(struct Thread* thr, struct Message** ptr) {
  ++(*ptr);
  if (*ptr == &(thr->messages[THREAD_MSG_QUEUE_SIZE])) {
    *ptr = &(thr->messages[0]);
  }
}

bool get_msg(int* sender, int* message) {
  if (current_thread->next_msg != current_thread->end_msgs
      || current_thread->msgs_full) {
    *sender = current_thread->next_msg->src;
    *message = current_thread->next_msg->content;

    inc_msg_pointer(current_thread, &current_thread->next_msg);
    current_thread->msgs_full = false;

    return true;
  }
  else {
    return false;
  }
}

bool send_msg(int destination, int message) {
  if (
      // Invalid destination
      destination >= MAX_THREADS ||
      destination < 0 ||
      all_threads[destination].id == -1 ||
      // Buffer is full
      all_threads[destination].msgs_full
  ) {
    return false;
  }

  struct Thread* dest = &all_threads[destination];
  struct Message* our_msg = dest->end_msgs;
  our_msg->src = get_thread_id();
  our_msg->content = message;
  inc_msg_pointer(dest, &(dest->end_msgs));
  dest->msgs_full = dest->next_msg == dest->end_msgs;

  return true;
}

void print_thread_id() {
  int tid = get_thread_id();
  if (tid == -1) {
    print("<HIDDEN>");
  } else {
    // Length matches the name above
    char out[9] = "        ";
    out[7] = (unsigned int)(tid)+48;
    print(out);
  }
}

void log_event(const char* event) {
  print("Thread "); print_thread_id(); print(": "); print(event); print("\n");
}

void thread_yield(struct Thread* to) {
  log_event("yielding");
  platform_yield((void**)&current_thread, to);
  log_event("resuming");
}

void yield() {
  // To be called in user threads
  thread_yield(&scheduler_thread);
}

__attribute__((noreturn)) void thread_start() {
  // Every thread starts by entering this function

  // Call thread's actual function
  current_thread->work();

  // Yield back to the scheduler
  log_event("exiting");

  // Mark this thread as invalid
  // Use ID to mark invalid threads instead of stack pointer
  // because we don't use the ID in yield, so it saves us
  // writing a custom yield for this situation.
  all_threads[get_thread_id()].id = -1;

  // Calling platform yield directly so we don't log_events
  // with an incorrect thread ID
  // TODO: we save state here that we don't need to
  platform_yield((void**)&current_thread, &scheduler_thread);

  __builtin_unreachable();
}

void init_thread(struct Thread* thread, int tid, void (*do_work)(void), bool hidden) {
  // thread start will jump to this
  thread->work = do_work;
  // but make sure thread start is the first call, so it can handle destruction
  thread->current_pc = thread_start;
  thread->stack_ptr = &(thread->stack[THREAD_STACK_SIZE-1]);
  thread->next_msg = &(thread->messages[0]);
  thread->end_msgs = thread->next_msg;
  thread->msgs_full = false;
  thread->id = tid;
}

int add_thread(void (*worker)(void)) {
  for (size_t idx=0; idx <= MAX_THREADS; ++idx) {
    if (all_threads[idx].id == -1) {
      init_thread(&all_threads[idx], idx, worker, false);
      return idx;
    }
  }
  return -1;
}

__attribute__((noreturn)) void do_scheduler() {
  while (1) {
    for (size_t idx=0; idx != MAX_THREADS; ++idx) {
      if (all_threads[idx].id != -1) {
        log_event("scheduling new thread");
        thread_yield(&all_threads[idx]);
        log_event("thread yielded");
      }  
    }
  } 
}

__attribute__((noreturn)) void start_scheduler() {
  // Hidden so that the scheduler doesn't run itself somehow
  init_thread(&scheduler_thread, -1, do_scheduler, true);

  // Need a dummy thread here otherwise we'll try to write to address 0
  struct Thread dummy;
  init_thread(&dummy, -1, (void (*)(void))(0), true);

  current_thread = &dummy;
  thread_yield(&scheduler_thread);

  __builtin_unreachable();
}

extern void demo();
__attribute__((noreturn)) void entry() {
  // Invalidate all threads in the pool
  for (size_t idx=0; idx < MAX_THREADS; ++idx) {
    all_threads[idx].id = -1;
  }

  // Call user app
  demo();

  __builtin_unreachable();
}

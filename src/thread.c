#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "thread.h"
#include "print.h"
#include "util.h"
#include "semihosting.h"

#define THREAD_STACK_SIZE 1024
#define THREAD_NAME_SIZE 12
#define THREAD_MSG_QUEUE_SIZE 5
#define STACK_CANARY 0xcafef00d

struct Message {
  int src;
  int content;
};

struct Thread {
  uint8_t* stack_ptr;
  void (*current_pc)(void);
  int id;
  const char* name;
  void (*work)(void);
  struct Message messages[THREAD_MSG_QUEUE_SIZE];
  struct Message* next_msg;
  struct Message* end_msgs;
  bool msgs_full;
  uint32_t canary;
  uint8_t stack[THREAD_STACK_SIZE];
};

extern void platform_yield(void);

__attribute__((section(".thread_structs")))
struct Thread all_threads[MAX_THREADS];

// Use these struct names to ensure that these are
// placed *after* the thread structs to prevent
// stack overflow corrupting them.
__attribute__((section(".thread_vars")))
struct Thread* current_thread;
__attribute__((section(".thread_vars")))
struct Thread* next_thread;
__attribute__((section(".thread_vars")))
struct Thread scheduler_thread;

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

bool is_valid_thread(int tid) {
  return (tid >= 0) &&
    (tid < MAX_THREADS) &&
    all_threads[tid].id != -1;
}

int get_thread_id() {
  return current_thread->id;
}

const char* get_thread_name() {
  return current_thread->name;
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
  char output[THREAD_NAME_SIZE+1];

  // fill with spaces (no +1 as we'll terminate it later)
  for (size_t idx=0; idx < THREAD_NAME_SIZE; ++idx) {
    output[idx] = ' ';
  }

  const char* name = current_thread->name;
  if (name == NULL) {
    int tid = get_thread_id();

    if (tid == -1) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_SIZE - h_len;
      strncpy(&output[padding], hidden, h_len);
    } else {
      // Length matches the name above
      output[THREAD_NAME_SIZE-1] = (unsigned int)(tid)+48;
    }
  } else {
    size_t name_len = name == NULL ? 0 : strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_SIZE) {
      name_len = THREAD_NAME_SIZE;
    }

    size_t padding = THREAD_NAME_SIZE-name_len;
    strncpy(&output[padding], name, name_len);
  }

  output[THREAD_NAME_SIZE] = '\0';
  print(output);
}

void log_event(const char* event) {
  print("Thread "); print_thread_id(); print(": "); print(event); print("\n");
}

void check_stack() {
  if (current_thread->canary != STACK_CANARY) {
    qemu_print("Stack overflow!\n");
    qemu_exit();
  }
}

void thread_yield(struct Thread* to) {
  check_stack();

  log_event("yielding");
  next_thread = to;
  platform_yield();
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
  next_thread = &scheduler_thread;
  platform_yield();

  __builtin_unreachable();
}

void init_thread(struct Thread* thread, int tid, const char* name,
                 void (*do_work)(void)) {
  // thread start will jump to this
  thread->work = do_work;
  // but make sure thread start is the first call so it can handle destruction
  thread->current_pc = thread_start;

  thread->id = tid;
  thread->name = name;

  // Start message buffer empty
  thread->next_msg = &(thread->messages[0]);
  thread->end_msgs = thread->next_msg;
  thread->msgs_full = false;

  thread->canary = STACK_CANARY;
  // Top of stack
  thread->stack_ptr = &(thread->stack[THREAD_STACK_SIZE-1]);
}

int add_thread(void (*worker)(void)) {
  return add_named_thread(worker, NULL);
}

int add_named_thread(void (*worker)(void), const char* name) {
  for (size_t idx=0; idx <= MAX_THREADS; ++idx) {
    if (all_threads[idx].id == -1) {
      init_thread(&all_threads[idx], idx, name, worker);
      return idx;
    }
  }
  return -1;
}

__attribute__((noreturn)) void do_scheduler() {
  while (1) {
    for (size_t idx=0; idx < MAX_THREADS; ++idx) {
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
  init_thread(&scheduler_thread, -1, "<scheduler>", do_scheduler);

  // Need a dummy thread here otherwise we'll try to write to address 0
  struct Thread dummy;
  init_thread(&dummy, -1, NULL, (void (*)(void))(0));

  current_thread = &dummy;
  thread_yield(&scheduler_thread);

  __builtin_unreachable();
}

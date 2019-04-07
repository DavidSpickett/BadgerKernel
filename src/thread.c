#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "thread.h"
#include "print.h"
#include "semihosting.h"

#define THREAD_STACK_SIZE 1024*STACK_SIZE
// 2 registers on AArch64
#define MONITOR_STACK_SIZE 2*8
#define THREAD_NAME_SIZE 12
#define THREAD_MSG_QUEUE_SIZE 5
#define STACK_CANARY 0xcafebeefdeadf00d

typedef struct {
  int src;
  int content;
} Message;

enum ThreadState {
  init=0,
  running=1,
  suspended=2,
  waiting,
  finished
};

typedef struct {
  uint8_t* stack_ptr;
  // Not an enum directly because we need to know its size
  size_t state;
  int id;
  const char* name;
  // Deliberately not (void)
  void (*work)();
  ThreadArgs args;
  Message messages[THREAD_MSG_QUEUE_SIZE];
  Message* next_msg;
  Message* end_msgs;
  bool msgs_full;
  uint64_t bottom_canary;
  uint8_t stack[THREAD_STACK_SIZE];
  uint64_t top_canary;
} Thread;

extern void thread_switch_initial(void);
extern void thread_switch(void);

__attribute__((section(".thread_structs")))
Thread all_threads[MAX_THREADS];
// Don't care if this gets corrupted, we'll just reset it's stack anyway
__attribute__((section(".thread_structs")))
static Thread dummy_thread;

__attribute__((section(".scheduler_thread")))
Thread scheduler_thread;

// Use these struct names to ensure that these are
// placed *after* the thread structs to prevent
// stack overflow corrupting them.
__attribute__((section(".thread_vars")))
Thread* current_thread;
__attribute__((section(".thread_vars")))
Thread* next_thread;
__attribute__((section(".thread_vars")))
size_t thread_stack_offset = offsetof(Thread, stack);

// Known good stack to save registers to while we check stack extent
// In a seperate section so we can garauntee it's alignement for AArch64
__attribute__((section(".monitor_vars")))
uint8_t monitor_stack[MONITOR_STACK_SIZE];
__attribute__((section(".thread_vars")))
uint8_t* monitor_stack_top = &monitor_stack[MONITOR_STACK_SIZE];

__attribute__((section(".thread_vars")))
MonitorConfig config = {
  destroy_on_stack_err: false,
  exit_when_no_threads: true
};

extern void demo(void);
__attribute__((noreturn)) void entry(void) {
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

bool is_thread_finished(int tid) {
  return is_valid_thread(tid) &&
    all_threads[tid].state == finished;
}

static bool can_schedule_thread(int tid) {
  return is_valid_thread(tid) &&
    all_threads[tid].state != finished &&
    all_threads[tid].state != waiting;
}

int get_thread_id(void) {
  return current_thread->id;
}

const char* get_thread_name(void) {
  return current_thread->name;
}

static void inc_msg_pointer(Thread* thr, Message** ptr) {
  ++(*ptr);
  // Wrap around from the end to the start
  if (*ptr == &(thr->messages[THREAD_MSG_QUEUE_SIZE])) {
    *ptr = &(thr->messages[0]);
  }
}

bool get_msg(int* sender, int* message) {
  // If message box is not empty, or it is full
  if (current_thread->next_msg != current_thread->end_msgs
      || current_thread->msgs_full) {
    *sender = current_thread->next_msg->src;
    *message = current_thread->next_msg->content;

    inc_msg_pointer(current_thread, &current_thread->next_msg);
    current_thread->msgs_full = false;

    return true;
  }

  return false;
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

  Thread* dest = &all_threads[destination];
  Message* our_msg = dest->end_msgs;
  our_msg->src = get_thread_id();
  our_msg->content = message;
  inc_msg_pointer(dest, &(dest->end_msgs));
  dest->msgs_full = dest->next_msg == dest->end_msgs;

  return true;
}

void print_thread_id(void) {
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
    size_t name_len = strlen(name);

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

void stack_extent_failed(void) {
  // current_thread is likely still valid here
  log_event("Not enough stack to save context!");
  qemu_exit();
}

void check_stack(void) {
  bool underflow = current_thread->bottom_canary != STACK_CANARY;
  bool overflow  = current_thread->top_canary != STACK_CANARY;

  if (underflow || overflow) {
    // Don't schedule this again, or rely on its ID
    current_thread->id = -1;
    current_thread->name = NULL;

    if (underflow) { log_event("Stack underflow!"); }
    if (overflow)  { log_event("Stack overflow!"); }

    if (config.destroy_on_stack_err) {
      /* Use the dummy thread to yield back to the scheduler
         without doing any more damage. */
      current_thread = &dummy_thread;

      /* Reset dummy's stack ptr so repeated exits here doesn't
         corrupt *that* stack. */
      dummy_thread.stack_ptr = &dummy_thread.stack[THREAD_STACK_SIZE];

      next_thread = &scheduler_thread;
      /* Setting -1 here, instead of state=finished is fine,
         because A: the thread didn't actually finish
                 B: the thread struct is actually invalid */
      current_thread->id = -1;
      thread_switch_initial();
    } else {
      qemu_exit();
    }
  }
}

void thread_yield(Thread* to) {
  check_stack();

  log_event("yielding");
  next_thread = to;
  thread_switch();
  log_event("resuming");
}

void yield(void) {
  // To be called in user threads
  thread_yield(&scheduler_thread);
}

bool yield_to(int tid) {
  if (!can_schedule_thread(tid)) {
    return false;
  }

  Thread* candidate = &all_threads[tid];
  thread_yield(candidate);
  return true;
}

bool yield_next(void) {
  // Yield to next valid thread, wrapping around the list
  int id = get_thread_id();

  // Don't call this in the scheduler
  if ((id == -1) || (id >= MAX_THREADS)) {
    return false;
  }

  // +1 otherwise we just schedule the current thread again
  int limit = id+MAX_THREADS+1;
  for (int idx=id+1; idx < limit; ++idx) {
    int idx_in_range = idx % MAX_THREADS;
    if (can_schedule_thread(idx_in_range)) {
      thread_yield(&all_threads[idx_in_range]);
      return true;
    }
  }

  // Not sure how you'd get here, you'd at least schedule yourself
  return false;
}

void thread_wait(void) {
  current_thread->state = waiting;
  // Call thread_switch directly to keep state intact
  next_thread = &scheduler_thread;
  thread_switch();
}

bool thread_wake(int tid) {
  if (is_valid_thread(tid)) {
    all_threads[tid].state = suspended;
    return true;
  }

  return false;
}

__attribute__((noreturn)) void thread_start(void) {
  // Every thread starts by entering this function

  // Call thread's actual function
  current_thread->work(
    current_thread->args.a1,
    current_thread->args.a2,
    current_thread->args.a3,
    current_thread->args.a4
  );

  // Yield back to the scheduler
  log_event("exiting");

  // Make sure we're not scheduled again
  current_thread->state = finished;

  /* You might think this is a timing issue.
     What if we're interrupted here?

     Well, we'd go to thread_switch, next_thread
     is set to the scheduler automatically.
     Since our state is finished, it won't be updated
     to suspended. Meaning, we'll never come back here.

     Which is just fine, since we were going to switch
     away anyway.
  */

  next_thread = &scheduler_thread;
  // Calling thread_switch directly so we don't print 'yielding'
  // TODO: we save state here that we don't need to
  thread_switch();

  __builtin_unreachable();
}

void init_thread(Thread* thread,
                 int tid,
                 const char* name,
                 void (*do_work)(void),
                 ThreadArgs args) {
  // thread_start will jump to this
  thread->work = do_work;
  thread->state = init;

  thread->id = tid;
  thread->name = name;
  thread->args = args;

  // Start message buffer empty
  thread->next_msg = &(thread->messages[0]);
  thread->end_msgs = thread->next_msg;
  thread->msgs_full = false;

  thread->bottom_canary = STACK_CANARY;
  thread->top_canary = STACK_CANARY;
  // Top of stack
  size_t stack_ptr = (size_t)(&(thread->stack[THREAD_STACK_SIZE]));
  // Mask to align to 16 bytes for AArch64
  thread->stack_ptr = (uint8_t*)(stack_ptr & ~0xF);
}

int add_named_thread_with_args(void (*worker)(),
                               const char* name,
                               ThreadArgs args) {
  for (size_t idx=0; idx <= MAX_THREADS; ++idx) {
    if (all_threads[idx].id == -1) {
      init_thread(&all_threads[idx], idx, name, worker, args);
      return idx;
    }
  }
  return -1;
}

int add_thread(void (*worker)()) {
  return add_named_thread(worker, NULL);
}

int add_named_thread(void (*worker)(), const char* name) {
  ThreadArgs args;
  return add_named_thread_with_args(worker, name, args);
}

__attribute__((noreturn)) void do_scheduler(void) {
  while (1) {
    bool live_threads = false;

    for (size_t idx=0; idx < MAX_THREADS; ++idx) {
      if (can_schedule_thread(idx)) {
        if (all_threads[idx].id != idx) {
          log_event("thread ID and position inconsistent!");
          qemu_exit();
        }

        log_event("scheduling new thread");
        live_threads = true;
        thread_yield(&all_threads[idx]);
        log_event("thread yielded");
      }  
    }

    if (!live_threads && config.exit_when_no_threads) {
      log_event("all threads finished");
      qemu_exit();
    }
  } 
}

__attribute__((noreturn)) void start_scheduler(void) {
  ThreadArgs args;

  // Hidden so that the scheduler doesn't run itself somehow
  init_thread(&scheduler_thread, -1, "<scheduler>", do_scheduler, args);

  // Need a dummy thread here otherwise we'll try to write to address 0
  init_thread(&dummy_thread, -1, NULL, (void (*)(void))(0), args);

  current_thread = &dummy_thread;
  next_thread = &scheduler_thread;
  log_event("starting scheduler");
  thread_switch_initial();

  __builtin_unreachable();
}

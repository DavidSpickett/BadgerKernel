#ifdef linux
#define _GNU_SOURCE
#include <pthread.h>
#endif
#include "print.h"
#include "thread.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef linux
#define THREAD_STACK_SIZE 1024 * STACK_SIZE
// 2 registers on AArch64
#define MONITOR_STACK_SIZE 2 * 8
#define STACK_CANARY       0xcafebeefdeadf00d
#endif

#define THREAD_NAME_SIZE      12
#define THREAD_MSG_QUEUE_SIZE 5

typedef struct {
  int src;
  int content;
} Message;

typedef struct {
#ifdef linux
  pthread_t self;
#else
  uint8_t* stack_ptr;
#endif
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
#ifndef linux
  uint64_t bottom_canary;
  uint8_t stack[THREAD_STACK_SIZE];
  uint64_t top_canary;
#endif
} Thread;

__attribute__((section(".thread_vars"))) Thread* _current_thread;

__attribute__((section(".thread_structs"))) Thread all_threads[MAX_THREADS];
// Volatile is here for the pthread implementation
__attribute__((section(".thread_vars"))) Thread* volatile next_thread;

__attribute__((section(".thread_vars")))
MonitorConfig config = {.destroy_on_stack_err = false,
                        .log_scheduler = true};

bool is_valid_thread(int tid) {
  return (tid >= 0) && (tid < MAX_THREADS) && all_threads[tid].id != -1;
}

static bool can_schedule_thread(int tid) {
  return is_valid_thread(tid) && (all_threads[tid].state == suspended ||
                                  all_threads[tid].state == init);
}

Thread* current_thread(void);

const char* get_thread_name(void) {
  return current_thread()->name;
}

int get_thread_id(void) {
  return current_thread()->id;
}

extern void setup(void);
void do_scheduler(void);
extern void start_thread_switch(void);
__attribute__((noreturn)) void entry(void) {
  // Invalidate all threads in the pool
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    all_threads[idx].id = -1;
  }

  // Call user setup
  setup();

#ifdef linux
  /* I don't think this is a race condition because
     next_thread is NULL until this sets it.
     The other threads will just yield back here
     until next_thread is set. */
  do_scheduler();
  // Let pthreads run
  while (1) { //!OCLINT
  }
#else
  // Already in kernel mode here
  start_thread_switch();
#endif

  __builtin_unreachable();
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
  if (current_thread()->next_msg != current_thread()->end_msgs ||
      current_thread()->msgs_full) {
    *sender = current_thread()->next_msg->src;
    *message = current_thread()->next_msg->content;

    inc_msg_pointer(current_thread(), &current_thread()->next_msg);
    current_thread()->msgs_full = false;

    return true;
  }

  return false;
}

bool send_msg(int destination, int message) {
  if (
      // Invalid destination
      destination >= MAX_THREADS || destination < 0 ||
      all_threads[destination].id == -1 ||
      // Buffer is full
      all_threads[destination].msgs_full) {
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

#ifdef linux
void thread_switch(void);
#else
extern void thread_switch(void);
void check_stack(void);
#endif

void thread_yield(Thread* next) {
  bool log = get_thread_id() != -1 || config.log_scheduler;

#ifndef linux
  check_stack();
#endif

  if (log) {
    log_event("yielding");
  }
  next_thread = next;
  thread_switch();
  if (log) {
    log_event("resuming");
  }
}

void yield(void) {
  // To be called in user threads
  thread_yield(NULL);
}

void format_thread_name(char* out) {
  // fill with spaces (no +1 as we'll terminate it later)
  for (size_t idx = 0; idx < THREAD_NAME_SIZE; ++idx) {
    out[idx] = ' ';
  }

  const char* name = current_thread()->name;
  
  if (name == NULL) {
    int tid = get_thread_id();
  
    if (tid == -1) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_SIZE - h_len;
      strncpy(&out[padding], hidden, h_len);
    } else {
      // Just show the ID number (assume max 999 threads)
      char idstr[4];
      int len = sprintf(idstr, "%u", tid);
      strcpy(&out[THREAD_NAME_SIZE - len], idstr);
    }
  } else {
    size_t name_len = strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_SIZE) {
      name_len = THREAD_NAME_SIZE;
    }

    size_t padding = THREAD_NAME_SIZE - name_len;
    strncpy(&out[padding], name, name_len);
  }

  out[THREAD_NAME_SIZE] = '\0';
}

void log_event(const char* event) {
  char thread_name[THREAD_NAME_SIZE + 1];
  format_thread_name(thread_name);
  printf("Thread %s: %s\n", thread_name, event);
}

void log_scheduler_event(const char* event) {
  if (config.log_scheduler) {
    printf("Thread  <scheduler>: %s\n", event);
  }
}

void do_scheduler(void) {
  size_t start_thread_idx;
  Thread* curr = current_thread();
  if (curr) {
    // +1 to skip the current thread
    start_thread_idx = curr - &all_threads[0] + 1;
  } else {
    // On startup current_thread will be NULL
    start_thread_idx = 0;
  }
  
  size_t max_thread_idx = start_thread_idx + MAX_THREADS;
  for (size_t idx = start_thread_idx; idx < max_thread_idx; ++idx) {
    // Wrap into range of array
    size_t _idx = idx % MAX_THREADS;

    if (!can_schedule_thread(_idx)) {
      continue;
    }

    if (all_threads[_idx].id != _idx) {
      printf("thread ID %u and position %u inconsistent!\n", (unsigned)_idx, all_threads[_idx].id);
      exit(1);
    }

    log_scheduler_event("scheduling new thread");

    log_scheduler_event("next thread chosen");
    next_thread = &all_threads[_idx];
    return; //!OCLINT
  }

  log_scheduler_event("all threads finished");
  exit(0);
}

static bool set_thread_state(int tid, ThreadState state) {
  if (is_valid_thread(tid)) {
    all_threads[tid].state = state;
    return true;
  }
  return false;
}

bool thread_wake(int tid) {
  return set_thread_state(tid, suspended);
}

bool thread_cancel(int tid) {
  return set_thread_state(tid, cancelled);
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

  // Check every other thread than this one
  size_t limit = id + MAX_THREADS;
  for (size_t idx = id + 1; idx < limit; ++idx) {
    size_t idx_in_range = idx % MAX_THREADS;
    if (can_schedule_thread(idx_in_range)) {
      thread_yield(&all_threads[idx_in_range]);
      return true;
    }
  }

  // Don't switch just continue to run current thread
  return false;
}

int add_thread(void (*worker)(void)) {
  return add_named_thread(worker, NULL);
}

int add_named_thread(void (*worker)(), const char* name) {
  ThreadArgs args = {0, 0, 0, 0};
  return add_named_thread_with_args(worker, name, args);
}

void init_thread(Thread* thread, int tid, const char* name,
                 void (*do_work)(void), ThreadArgs args) {
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

#ifndef linux
  thread->bottom_canary = STACK_CANARY;
  thread->top_canary = STACK_CANARY;
  // Top of stack
  size_t stack_ptr = (size_t)(&(thread->stack[THREAD_STACK_SIZE]));
  // Mask to align to 16 bytes for AArch64
  thread->stack_ptr = (uint8_t*)(stack_ptr & ~0xF);
#endif
}

#ifdef linux
void* thread_entry();
#endif
int add_named_thread_with_args(void (*worker)(), const char* name,
                               ThreadArgs args) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    if (all_threads[idx].id == -1) {
      init_thread(&all_threads[idx], idx, name, worker, args);

#ifdef linux
      pthread_create(&all_threads[idx].self, NULL, thread_entry, NULL);
#endif

      return idx;
    }
  }
  return -1;
}

void thread_wait(void) {
  current_thread()->state = waiting;
  // Skip the yielding print
  next_thread = NULL;
  thread_switch();
}

bool thread_join(int tid, ThreadState* state) {
  while (1) {
    // Initial ID is invalid, or it was destroyed due to stack err
    if (!is_valid_thread(tid)) {
      return false;
    }

    ThreadState ts = all_threads[tid].state;
    if (ts == finished || ts == cancelled) {
      if (state) {
        *state = ts;
      }
      return true;
    } else {
      yield();
    }
  }
}

#ifdef linux

void thread_switch_alrm() {
  if (!current_thread()) {
    // Something other than a user thread caught the signal
    printf("Rejecting alrm\n");
    return;
  }  
  next_thread = NULL;
  thread_switch();
}

Thread* current_thread(void) {
  pthread_t self = pthread_self();
  for (int i = 0; i < MAX_THREADS; ++i) {
    if (all_threads[i].self == self) {
      return &all_threads[i];
    }
  }
  // Main thread should only hit this once when it
  // calls do_scheduler for the first time.
  static bool called_on_main = false;
  if (called_on_main) __builtin_unreachable();
  called_on_main = true;
  return NULL;
}

void* thread_entry() {
  while (next_thread != current_thread()) {
    pthread_yield();
  }

  current_thread()->work(current_thread()->args.a1, current_thread()->args.a2,
                         current_thread()->args.a3, current_thread()->args.a4);

  // Yield back to the scheduler
  log_event("exiting");

  // Make sure we're not scheduled again
  current_thread()->state = finished;

  // TODO: free all heap allocations

  // Must call this so we check if there are threads left
  do_scheduler();

  return NULL; //!OCLINT
}

void thread_switch(void) {
  if (!next_thread) {
    do_scheduler();
  } 
  while (next_thread != current_thread()) {
    pthread_yield();
  }
}

#else

// Use these struct names to ensure that these are
// placed *after* the thread structs to prevent
// stack overflow corrupting them.
__attribute__((section(".thread_vars"))) size_t thread_stack_offset =
    offsetof(Thread, stack);

Thread* current_thread(void) {
  return _current_thread;
}

void stack_extent_failed(void) {
  // current_thread is likely still valid here
  log_event("Not enough stack to save context!");
  exit(1);
}

void check_stack(void) {
  bool underflow = current_thread()->bottom_canary != STACK_CANARY;
  bool overflow = current_thread()->top_canary != STACK_CANARY;

  if (underflow || overflow) {
    // Don't schedule this again, or rely on its ID
    current_thread()->id = -1;
    current_thread()->name = NULL;

    if (underflow) {
      log_event("Stack underflow!");
    }
    if (overflow) {
      log_event("Stack overflow!");
    }

    if (config.destroy_on_stack_err) {
     /* Setting -1 here, instead of state=finished is fine,
         because A: the thread didn't actually finish
                 B: the thread struct is actually invalid */
      current_thread()->id = -1;

      // Aka don't save any state, just load the scheduler
      current_thread()->state = init;
      thread_switch();
    } else {
      exit(1);
    }
  }
}

__attribute__((noreturn)) void thread_start(void) {
  // Every thread starts by entering this function

  // Call thread's actual function
  current_thread()->work(current_thread()->args.a1, current_thread()->args.a2,
                         current_thread()->args.a3, current_thread()->args.a4);

  // Yield back to the scheduler
  log_event("exiting");

  // Make sure we're not scheduled again
  current_thread()->state = finished;

  /* You might think this is a timing issue.
     What if we're interrupted here?

     Well, we'd go to thread_switch, next_thread
     is set to the scheduler automatically.
     Since our state is finished, it won't be updated
     to suspended. Meaning, we'll never come back here.

     Which is just fine, since we were going to switch
     away anyway.
  */

  // Calling thread_switch directly so we don't print 'yielding'
  // TODO: we save state here that we don't need to
  thread_switch();

  __builtin_unreachable();
}

#endif // ifndef linux

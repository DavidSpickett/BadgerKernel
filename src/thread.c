#include "thread.h"
#include "print.h"

#define MAX_THREADS 10

static struct Thread scheduler_thread;
static struct Thread* all_threads[MAX_THREADS];
static struct Thread* current_thread;

int get_thread_id() {
  return current_thread->id;
}

void print_thread_id() {
  int id = get_thread_id();
  switch (id) {
    case -1:
      print("<HIDDEN>");
      break;
    default:
    {
      // Length matches the name above
      char* o = "        ";
      o[7] = (unsigned int)(id)+48;
      print(o);
      break;
    }
  }
}

void log_event(const char* event) {
  print("Thread "); print_thread_id(); print(": "); print(event); print("\n");
}

void thread_yield(struct Thread* to) {
  log_event("yielding");
  #include "yield.inc"
  log_event("resuming");
}

void yield() {
  // To be called in user threads
  thread_yield(&scheduler_thread);
}

int add_thread(struct Thread* new_thread) {
  for (size_t idx=0; idx != MAX_THREADS; ++idx) {
    if (!all_threads[idx]) {
      all_threads[idx] = new_thread;
      return idx;
    }
  }
  return -1;
}

void init_thread(struct Thread* thread, void (*do_work)(void), bool hidden) {
  thread->current_pc = do_work;
  thread->stack_ptr = &(thread->stack[THREAD_STACK_SIZE-1]);
  // TODO: handle err
  thread->id = hidden ? -1 : add_thread(thread);
}

__attribute__((noreturn)) void do_scheduler() {
  while (1) {
    for (size_t idx=0; idx != MAX_THREADS; ++idx) {
      if (all_threads[idx]) {
        log_event("scheduling new thread");
        thread_yield(all_threads[idx]);
        log_event("thread yielded");
      }  
    }
  } 
}

__attribute__((noreturn)) void start_scheduler() {
  // Hidden so that the scheduler doesn't run itself somehow
  init_thread(&scheduler_thread, do_scheduler, true);
  // Need a dummy thread here otherwise we'll try to write to address 0
  struct Thread dummy;
  init_thread(&dummy, (void (*)(void))(0), true);
  current_thread = &dummy;
  thread_yield(&scheduler_thread);
  __builtin_unreachable();
}

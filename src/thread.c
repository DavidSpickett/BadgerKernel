#include "thread.h"
#include "print.h"

#define MAX_THREADS 10

extern void platform_yield(void**, void*);

static struct Thread scheduler_thread;
static struct Thread* all_threads[MAX_THREADS];
static struct Thread* current_thread;

int get_thread_id() {
  return current_thread->id;
}

static void inc_msg_pointer(struct Thread* thr, struct Message** ptr) {
  // wrap around the end back to the start
  if (*ptr == &(thr->messages[THREAD_MSG_QUEUE_SIZE-1])) {
    *ptr = &(thr->messages[0]);
  // otherwise go forward normally
  } else {
    *ptr += 1;
  }
}

bool get_msg(int* sender, int* message) {
  if (current_thread->next_msg != current_thread->end_msgs) {
    *sender = current_thread->next_msg->src;
    *message = current_thread->next_msg->content;

    inc_msg_pointer(current_thread, &current_thread->next_msg);
    return true;
  }
  else {
    return false;
  }
}

bool send_msg(int destination, int message) {
  // Invalid destination thread
  if (destination >= MAX_THREADS ||
      all_threads[destination] == NULL) {
    return false;
  }

  // Thread's message box is full
  struct Thread* dest = all_threads[destination];
  struct Message* test_ptr = dest->end_msgs;
  inc_msg_pointer(dest, &test_ptr);
  if (test_ptr == dest->next_msg) {
    return false;
  }

  // Now we can send something
  struct Message * our_msg = dest->end_msgs;
  our_msg->src = get_thread_id();
  our_msg->content = message;
  inc_msg_pointer(dest, &(dest->end_msgs));

  return true;
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
  platform_yield((void**)&current_thread, to);
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
  thread->next_msg = &(thread->messages[0]);
  thread->end_msgs = thread->next_msg;
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

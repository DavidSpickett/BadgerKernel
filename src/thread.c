#include "thread.h"
#include "print.h"

extern void platform_yield(void**, void*);

static struct Thread scheduler_thread;
static struct Thread* current_thread;

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

  // Thread's message box is full if there is
  // a one message gap between the end of the list
  // and the start.
  // If end and start are the same, it's an empty list
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

  // Then mark this thread as invalid
  all_threads[get_thread_id()] = NULL;

  // Yield back to the scheduler
  // Note that we NULLed the all_threads ptr but
  // current_thread is still valid.
  // TODO: we save state here that we don't need to
  yield();

  __builtin_unreachable();
}

int add_thread(struct Thread* new_thread) {
  for (size_t idx=0; idx < MAX_THREADS; ++idx) {
    if (!all_threads[idx]) {
      all_threads[idx] = new_thread;
      return idx;
    }
  }
  return -1;
}

void init_thread(struct Thread* thread, void (*do_work)(void), bool hidden) {
  // thread start will jump to this
  thread->work = do_work;
  // but make sure thread start is the first call, so it can handle destruction
  thread->current_pc = thread_start;
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

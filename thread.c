#include "thread.h"

#define MAX_THREADS 10

static struct Thread scheduler_thread;
static struct Thread* all_threads[MAX_THREADS];
static struct Thread* current_thread;

void print(const char *s) {
  volatile unsigned int * const UART0DR = (unsigned int *)0x101f1000;
  while(*s != '\0') { 
    *UART0DR = (unsigned int)(*s++);
  }
}

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

  // TODO: CPSR/SPSR, flags?
  asm volatile (
                /* Save our own state */
                "push {r0-r12, r14}\n\t"      // note no PC here
                "ldr r1, =current_thread\n\t" // get *address* of current thread var
                "ldr r1, [r1]\n\t"            // get actual adress of current thread
                "str sp, [r1], #4\n\t"        // save current stack pointer to struct
                "ldr r2, =thread_return\n\t"  // get address to resume this thread from
                "str r2, [r1]\n\t"            // make that our next new PC
                
                /* Switch to new thread */
                "mov r1, %0\n\t"              // get addr of thread to move to (directly this time)
                "ldr r2, =current_thread\n\t" // set new current thread addr
                "str r1, [r2]\n\t"            // (don't need to chase ptr here)
                "ldr sp, [r1], #4\n\t"        // restore stack pointer of new thread
                "ldr pc, [r1]\n\t"            // jump to the next PC of that thread

                /* Resume this thread */
                "thread_return:\n\t"          // threads are resumed from here
                "ldr r1, =current_thread\n\t" // current will always be ourselves here
                "ldr r1, [r1]\n\t"            // get *value* of current thread ptr
                "ldr sp, [r1], #4\n\t"        // restore our own stack pointer
                "pop {r0-r12, r14}\n\t"       // restore our own regs
               :
               : "r" (to)
               );
  
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
  current_thread = &dummy;
  thread_yield(&scheduler_thread);
  __builtin_unreachable();
}

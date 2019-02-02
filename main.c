#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define THREAD_STACK 512
#define MAX_THREADS 10

volatile unsigned int * const UART0DR = (unsigned int *)0x101f1000;
 
void print_uart0(const char *s) {
 while(*s != '\0') { /* Loop until end of string */
 *UART0DR = (unsigned int)(*s); /* Transmit char */
 s++; /* Next char */
 }
}

struct Thread {
  uint8_t* stack_ptr;
  void (*current_pc)(void);
  uint8_t stack[THREAD_STACK];
  int id;
} scheduler_thread;

struct Thread* all_threads[MAX_THREADS];
struct Thread* current_thread;

int get_thread_id() {
  return current_thread->id;
}

void print_thread_id() {
  int id = get_thread_id();
  switch (id) {
    case -1:
      print_uart0("<HIDDEN>");
      break;
    default:
    {
      // Length matches the name above
      char* o = "        ";
      o[7] = (unsigned int)(id)+48;
      print_uart0(o);
      break;
    }
  }
}

void log_thread_event(const char* event) {
  print_uart0("Thread "); print_thread_id(); print_uart0(": "); print_uart0(event); print_uart0("\n");
}

void thread_yield(struct Thread* to) {
  log_thread_event("yielding");

  asm volatile ("push {r0-r12, r14}\n\t"      // No PC here
                "ldr r1, =current_thread\n\t" // get *address* of current thread var
                "ldr r1, [r1]\n\t"            // get actual adress of running thread
                "str sp, [r1], #4\n\t"       // save current stack pointer to struct
                "ldr r2, =user_thread_return\n\t"  // get address to resume from
                "str r2, [r1]\n\t"            // make that our new PC
                
                "mov r1, %0\n\t" // get addr of thread to move to
                "ldr sp, [r1], #4\n\t"         // restore stack pointer
                "ldr pc, [r1]\n\t"             // jump back to scheduler

                "user_thread_return:\n\t"
                "ldr r1, =current_thread\n\t" // Restore state
                "ldr r1, [r1]\n\t" // get actual adress of current thread
                "ldr sp, [r1], #4\n\t" //restore sp
                "pop {r0-r12, r14}\n\t" //restore regs
               :
               : "r" (to)
               );
  
  log_thread_event("resuming");
}

void yield() {
  thread_yield(&scheduler_thread);
}

__attribute__((noreturn)) void thread_work() {
  while (1) {
    log_thread_event("work part 1");
    yield();
    log_thread_event("work part 2");
    yield();
  }
}

void start_thread(struct Thread* thread) {
  current_thread = thread;

  // Ignore CPSR and SPSR for now since we're going to be manully yielding
  asm volatile ("push {r0-r12, r14}\n\t"     // note: no PC here
                "ldr r1, =scheduler_thread\n\t"
                "str sp, [r1], #4\n\t" // save current sp to thread struct
                "ldr r2, =thread_return\n\t" // when a thread yields it will return to this label
                "str r2, [r1]\n\t"   // now a yielding thread returns to thread_return

                "ldr r1, =current_thread\n\t" // current thread is already the user thread
                "ldr r1, [r1]\n\t" // get value of ptr
                "ldr sp, [r1], #4\n\t" // load up thread's stack ptr and inc
                "ldr pc, [r1]\n\t" // jump into thread

                // Thread does work here...

                "thread_return:\n\t" // other thread must have yielded
                "ldr r1, =scheduler_thread\n\t"
                "ldr sp, [r1]\n\t"  // Get our correct sp back
                "pop {r0-r12, r14}" // Get the rest of our regs back
               :
               : "r" (thread)
               ); 
  
  current_thread = &scheduler_thread;
}

__attribute__((noreturn)) void start_scheduler() {
  while (1) {
    for (size_t idx=0; idx != MAX_THREADS; ++idx) {
      if (all_threads[idx]) {
        log_thread_event("scheduling new thread");
        start_thread(all_threads[idx]);
        log_thread_event("thread yielded");
      }  
    }
  } 
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
  thread->stack_ptr = &(thread->stack[THREAD_STACK-1]);
  // TODO: handle err
  thread->id = hidden ? -1 : add_thread(thread);
}


__attribute__((noreturn)) void main() {
  // Hidden so that the scheduler doesn't run itself somehow
  init_thread(&scheduler_thread, start_scheduler, true);

  struct Thread thread1;
  init_thread(&thread1, thread_work, false);
  struct Thread thread2;
  init_thread(&thread2, thread_work, false);

  current_thread = &scheduler_thread;
  asm volatile ("ldr r1, =scheduler_thread\n\t" // Load scheduler thread address
                "ldr sp, [r1]\n\t"              // Set sp to scheduler thread's stack
                "add r1, r1, #4\n\t"            // Point r1 to the work function pointer
                "ldr pc, [r1]\n\t"              // "jump" to that address
               );
  __builtin_unreachable();
}

void c_entry() {
  main();
}


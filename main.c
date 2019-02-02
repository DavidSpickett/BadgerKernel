#include <stdint.h>
#include <stddef.h>

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
} scheduler_thread;

struct Thread* all_threads[MAX_THREADS];

__attribute__((noreturn)) void thread1_work() {
  while (1) {
    print_uart0("Thread 1!\n");
    /*
    Emit a message
    yield
    */
  }
}

__attribute__((noreturn)) void thread2_work() {
  while (1) {
    /*
    Emit a message
    yield
    */
  }
}

void start_thread(struct Thread* thread) {
  // Ignore CPSR and SPSR for now since we're going to be manully yielding

  // Push the sp manually to avoid asm warnings  
  asm volatile ("push {r0-r12, r14}\n\t"     // note: no PC here
                "ldr r1, =scheduler_thread\n\t"
                "str sp, [r1]\n\t" // save current sp to thread struct
                "add r1, #4\n\t"   // move to fn pointer
                "ldr r2, =thread_return\n\t" // when a thread yields it will return to this label
                "str r2, [r1]\n\t"   // now a yielding thread returns to thread_return

                // At this point:
                // * registers apart from PC are on scheduler's stack
                // * stack_ptr in struct is updated to the current sp
                // * function pointer points to where a yielding thread will return

                "mov r1, %0\n\t" // get address of thread
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

  // return to the scheduling loop...
}

__attribute__((noreturn)) void start_scheduler() {
  while (1) {
    for (size_t idx=0; idx != MAX_THREADS; ++idx) {
      if (all_threads[idx]) {
        start_thread(all_threads[idx]);
      }  
    }
  } 
}

int add_thread(struct Thread* new_thread) {
  for (size_t idx=0; idx != MAX_THREADS; ++idx) {
    if (!all_threads[idx]) {
      all_threads[idx] = new_thread;
      return 0;
    }
  }
  return -1;
}

void init_thread(struct Thread* thread, void (*do_work)(void)) {
  thread->current_pc = do_work;
  thread->stack_ptr = &(thread->stack[THREAD_STACK-1]);
}


__attribute__((noreturn)) void main() {
  init_thread(&scheduler_thread, start_scheduler);

  struct Thread thread1;
  init_thread(&thread1, thread1_work);
  add_thread(&thread1);
  struct Thread thread2;
  init_thread(&thread2, thread2_work);
  add_thread(&thread2);

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


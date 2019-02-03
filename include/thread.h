#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define THREAD_STACK_SIZE 512
struct Thread {
  uint8_t* stack_ptr;
  void (*current_pc)(void);
  uint8_t stack[THREAD_STACK_SIZE];
  int id;
};

void init_thread(struct Thread* thread, void (*do_work)(void), bool hidden);
int get_thread_id();
void yield();
void __attribute__((noreturn)) start_scheduler();
void log_event(const char* event);

#endif /* ifdef THREAD_H */

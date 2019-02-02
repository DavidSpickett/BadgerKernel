#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define THREAD_STACK 512

struct Thread {
  uint8_t* stack_ptr;
  void (*current_pc)(void);
  uint8_t stack[THREAD_STACK];
  int id;
};

int get_thread_id();
void log_thread_event(const char* event);
void yield();
void init_thread(struct Thread* thread, void (*do_work)(void), bool hidden);
void __attribute__((noreturn)) start_scheduler();
void log_thread_event(const char* event);

#endif /* ifdef THREAD_H */
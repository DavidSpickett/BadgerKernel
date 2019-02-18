#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define THREAD_STACK_SIZE 512
// +1 because we need a gap to show the 'full' state
#define THREAD_MSG_QUEUE_SIZE 5+1

struct Message {
  int src;
  int content;
};

struct Thread {
  uint8_t* stack_ptr;
  void (*current_pc)(void);
  void (*work)(void);
  uint8_t stack[THREAD_STACK_SIZE];
  struct Message messages[THREAD_MSG_QUEUE_SIZE];
  struct Message* next_msg;
  struct Message* end_msgs;
  int id;
};

#define MAX_THREADS 10
struct Thread* all_threads[MAX_THREADS];

void init_thread(struct Thread* thread, void (*do_work)(void), bool hidden);
int get_thread_id();
void yield();
void __attribute__((noreturn)) start_scheduler();
void log_event(const char* event);
bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef THREAD_H */

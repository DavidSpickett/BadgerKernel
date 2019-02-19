#ifndef THREAD_H
#define THREAD_H

#include <stdbool.h>

#define MAX_THREADS 10

int add_thread(void (*worker)(void));
bool is_valid_thread(int tid);
int get_thread_id();
void yield();
void __attribute__((noreturn)) start_scheduler();
void log_event(const char* event);
bool get_msg(int* sender, int* message);
bool send_msg(int destination, int message);

#endif /* ifdef THREAD_H */

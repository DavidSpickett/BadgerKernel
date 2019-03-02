#include <string.h>
#include "thread.h"
#include "mutex.h"

struct Mutex buffer_mutex;
char buffer[4];

void thread_work(const char* word) {
  while (!lock_mutex(&buffer_mutex)) {
    yield_next();
  }

  /* Looks a little silly, but shows we can
     yield during the copy with no issues. */
  char* dest = buffer;
  while (*word) {
    *dest++ = *word++;
    log_event("copying...");
    yield_next();
  } 

  log_event(buffer);
  unlock_mutex(&buffer_mutex);
}

void demo() {
  init_mutex(&buffer_mutex);
  
  const char* word1 = "dog";
  struct ThreadArgs a1 = make_args(word1, 0, 0, 0);
  add_named_thread_with_args(thread_work, NULL, a1);

  const char* word2 = "cat";
  struct ThreadArgs a2 = make_args(word2, 0, 0, 0);
  add_named_thread_with_args(thread_work, NULL, a2);

  start_scheduler(); 
}

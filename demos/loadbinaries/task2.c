#include "thread.h"
#include "print.h"

char buf[64];
__attribute__((section(".worker")))
void worker() {
  sprintf(buf, "from thread %u", get_thread_id());
  log_event("Greetings %s!", buf);
}

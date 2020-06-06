#include "user/thread.h"
#include "print.h"

char buf[64];
void worker() {
  sprintf(buf, "from thread %u", get_thread_id());
  log_event("Greetings %s!", buf);
}

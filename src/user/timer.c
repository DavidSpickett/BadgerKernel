#include "user/timer.h"
#include "common/svc_calls.h"
#include "port/port.h"
#include <stddef.h>

void enable_timer() {
  do_svc(svc_enable_timer);
}

void disable_timer() {
  do_svc(svc_disable_timer);
}

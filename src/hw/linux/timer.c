#include "timer.h"
#include <unistd.h>

void enable_timer() {
  // TODO: it would be bad to yield() before the alarm
  // went off (though it might work)
  ualarm(50, 0);
}

void disable_timer() {
  // Cancels any pending alarm
  ualarm(0, 0);
}

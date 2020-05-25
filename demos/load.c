#include "thread.h"

void loader() {
  add_thread_from_file("loadable.bin");
}

void setup(void) {
  add_thread(loader);
}

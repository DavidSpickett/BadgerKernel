#include "thread.h"
#include "util.h"
#include <stddef.h>

void printer(int repeat, char* phrase, int sub_printer, int start) {
  for (; repeat; --repeat) {
    log_event(phrase);
  }
  // Tell the next thread where to start
  send_msg(sub_printer, start);
}

void sub_printer(char** words, int num_phrases, unsigned int offset) {
  int sender, start;
  bool got = get_msg(&sender, &start);
  ASSERT(got);

  for (int idx = start; idx != num_phrases; ++idx) {
    log_event(words[idx] + offset);
  }
}

static const char* words[] = {"food", "alligator", "magazine", "raptor"};

void setup(void) {
  config.log_scheduler = false;

  ThreadArgs p_args = make_args(3, "aardvark", 1, 2);
  add_named_thread_with_args(printer, "printer", p_args);

  // Only takes 3, just zero the 4th arg
  ThreadArgs sp_args = make_args(words, 4, 2, 0);
  add_named_thread_with_args(sub_printer, "sub_printer", sp_args);
}

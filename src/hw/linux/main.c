#include <signal.h>
#include <stdlib.h>

extern void entry(void);
extern void thread_switch_alrm();

int main() {
  struct sigaction sa;
  sa.sa_handler = thread_switch_alrm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_NODEFER;

  int res = sigaction(SIGALRM, &sa, NULL);
  if (res != 0) {
    exit(1);
  }

  entry();
  return 0;
}

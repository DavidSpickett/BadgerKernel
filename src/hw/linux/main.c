#include <signal.h>
#include <stdlib.h>

extern void entry(void);
extern void thread_switch_alrm();

int main() {
  struct sigaction sact;
  sact.sa_handler = thread_switch_alrm;
  sigemptyset(&sact.sa_mask);
  sact.sa_flags = SA_NODEFER;

  int res = sigaction(SIGALRM, &sact, NULL);
  if (res != 0) {
    exit(1);
  }

  entry();
  return 0;
}

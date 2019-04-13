#include <signal.h>
#include <stdlib.h>

extern void entry(void);
extern void thread_switch_alrm();

int main() {
  struct sigaction sac;
  sac.sa_handler = thread_switch_alrm;
  sigemptyset(&sac.sa_mask);
  sac.sa_flags = SA_NODEFER;

  int res = sigaction(SIGALRM, &sac, NULL);
  if (res != 0) {
    exit(1);
  }

  entry();
  return 0;
}

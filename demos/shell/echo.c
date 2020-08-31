#include "common/print.h"

void worker(int argc, char* argv[]) {
  if (argc <= 1) {
    return;
  }
  for (int i = 1; i < argc; ++i) {
    printf("%s ", argv[i]);
  }
  printf("\n");
}

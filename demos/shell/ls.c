#include "common/print.h"
#include "user/file.h"

#define MAX_RES 1024

void worker(int argc, char* argv[]) {
  if (argc > 2) {
    printf("ls expects at most one path argument\n");
    return;
  }

  char res[MAX_RES];
  const char* path = NULL;
  if (argc == 2) {
    path = argv[1];
  } else {
    path = ".";
  }

  int ret = list_dir(path, res, MAX_RES);
  if (ret != 0) {
    printf("Couldn't ls %s, error %i\n", path, ret);
    return;
  }

  printf("%s", res);
}

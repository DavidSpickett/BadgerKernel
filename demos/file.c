/* Hello file demo! */
#include "thread.h"
#include "util.h"
#include "print.h"

void read_file(void) {
  char* path = "demos/file.c";
  int file = open(path, O_RDONLY);
  if (!file) {
    print("Failed to open %s\n", path);
    exit(1);
  }

  size_t buf_sz = 100;
  char content[buf_sz];
  read(file, content, buf_sz);

  char* ptr = content;
  while (*ptr != '\n')
  {
    ptr++;
  }
  *ptr = '\0';

  log_event(content);
  int failed = close(file);
  if (failed) {
    print("Failed to close file %s\n", path);
    exit(1);
  }
}

void setup(void) {
  config.log_scheduler = false;

  add_named_thread(read_file, "reader");
}

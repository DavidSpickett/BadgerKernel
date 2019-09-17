/* Hello file demo! */
#include "print.h"
#include "thread.h"
#include "util.h"
#include <string.h>

void fail_open(void) {
  int badfile = open("demos/not_a_demo.xyz", O_RDONLY);
  ASSERT(badfile == -1);
}

void read_file(void) {
  log_event("Reading demo src");
  const char* path = "demos/file.c";
  int file = open(path, O_RDONLY);
  ASSERT(file != -1);

  size_t buf_sz = 100;
  char content[buf_sz];
  read(file, content, buf_sz);

  char* ptr = content;
  while (*ptr != '\n') {
    ptr++;
  }
  *ptr = '\0';

  log_event(content);
  ASSERT(!close(file));
}

const char* temp_file = "demos/file_demo_temp_file";
const char* temp_contents = "new file for file demo!\n";
void write_new(void) {
  log_event("Writing temp file");
  int newfile = open(temp_file, O_WRONLY | O_CREAT, S_IRUSR);
  ASSERT(newfile != -1);
  size_t len = strlen(temp_contents);
  size_t ret = write(newfile, temp_contents, len);
  ASSERT(ret == len);
  ASSERT(!close(newfile));
}

void read_new(void) {
  log_event("Reading temp file");
  int newfile = open(temp_file, O_RDONLY);
  ASSERT(newfile != -1);
  size_t len = strlen(temp_contents);
  char got[len + 1];
  size_t ret = read(newfile, got, len);
  ASSERT(ret == len);
  got[len] = '\0';
  printf("%s", got);
  ASSERT(!close(newfile));
}

void delete_new(void) {
  log_event("Removing temp file");
  ASSERT(!remove(temp_file));
  ASSERT(remove(temp_file) == -1);
}

void setup(void) {
  config.log_scheduler = false;

  add_named_thread(read_file, "reader");
  add_named_thread(fail_open, "fail_open");
  add_named_thread(write_new, "write_new");
  add_named_thread(read_new, "read_new");
  add_named_thread(delete_new, "delete_new");
}

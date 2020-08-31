/* Hello file demo! */
// This demo is for semihosting file access
#include "user/file.h"
#include "common/print.h"
#include "user/thread.h"
#include "util.h"
#include <string.h>

void fail_open(void) {
  int badfile = open("demos/not_a/demo.xyz", O_RDONLY);
  assert(badfile == -1);
}

void read_file(void) {
  log_event("Reading demo src");
  const char* path = "demos/file/file.c";
  int file = open(path, O_RDONLY);
  assert(file != -1);

  size_t buf_sz = 100;
  char content[buf_sz];
  size_t got = read(file, content, buf_sz);
  assert(got == buf_sz);

  char* ptr = content;
  while (*ptr != '\n') {
    ptr++;
  }
  *ptr = '\0';

  log_event(content);
  int closed = close(file);
  assert(!closed);
}

const char* temp_file = "demos/file/file_demo_temp_file";
const char* temp_contents = "new file for file demo!\n";
void write_new(void) {
  log_event("Writing temp file");
  int newfile = open(temp_file, O_WRONLY);
  assert(newfile != -1);
  size_t len = strlen(temp_contents);
  size_t ret = write(newfile, temp_contents, len);
  assert(ret == len);
  int closed = close(newfile);
  assert(!closed);
}

void read_new(void) {
  log_event("Reading temp file");
  int newfile = open(temp_file, O_RDONLY);
  assert(newfile != -1);
  size_t len = strlen(temp_contents);
  char got[len + 1];
  size_t ret = read(newfile, got, len);
  assert(ret == len);
  got[len] = '\0';
  printf("%s", got);
  int closed = close(newfile);
  assert(!closed);
}

void delete_new(void) {
  log_event("Removing temp file");
  int removed = remove(temp_file);
  assert(!removed);
  removed = remove(temp_file);
  assert(removed == -1);
}

void setup(void) {
  add_named_thread(read_file, "reader");
  add_named_thread(fail_open, "fail_open");
  add_named_thread(write_new, "write_new");
  add_named_thread(read_new, "read_new");
  add_named_thread(delete_new, "delete_new");
}

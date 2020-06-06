// This demo is for the in memory file system
#include "alloc.h"
#include "user/thread.h"
#include "thread.h"
#include "file_system.h"
#include "util.h"
#include <string.h>

void errors(void) {
  // Can't open a non existent file
  assert(open("/bad/path/here", O_RDONLY) == -1);

  // Can't write to invalid FD
  assert(!write(-1, NULL, 0));

  // Won't write anything from a null buffer
  assert(!write(1, NULL, 0));

  // Can't close an invalid FD
  assert(close(-10) == -1);
  assert(close(99) == -1);

  // Can't remove a non existent file
  assert(remove("/not/a/file") == -1);
}

__attribute__((
  annotate("oclint:suppress[high cyclomatic complexity]"),
  annotate("oclint:suppress[high ncss method]")))
void basics(void) {
  // Make a new file
  char* file_path = "/bar/foo.txt";
  int handle = open(file_path, O_WRONLY);
  assert(handle != -1);

  // Write some real content
  const char* content = "The quick brown fox.";
  size_t write_sz = strlen(content) + 1;
  size_t wrote = write(handle, content, write_sz);
  assert(wrote == write_sz);

  // Allows us to close
  assert(!close(handle));

  // Open something else to re-use the FD
  const char* temp_path = "/a/b/c/d";
  int temp_handle = open(temp_path, O_WRONLY);
  assert(temp_handle != -1);
  assert(temp_handle == handle);

  // Open for reading
  int got_handle = open(file_path, O_RDONLY);
  // Should give a different FD
  assert(got_handle != handle);

  // Read back content and compare
  char got[write_sz];
  size_t read_sz = read(got_handle, got, write_sz);
  assert(read_sz == write_sz);
  assert(!strcmp(content, got));

  assert(!close(temp_handle));
  assert(!close(got_handle));
  remove(temp_path);

  // Remove test file
  assert(!remove(file_path));
  // Can't open a removed file
  assert(open(file_path, O_RDONLY) == -1);
}

void list_files(void) {
#define MAKE_PATH(path) open(path, O_WRONLY)
  MAKE_PATH("/l1a");
  MAKE_PATH("/f0/l2a");
  MAKE_PATH("/l1b");
  MAKE_PATH("/f1/l2b");
  MAKE_PATH("/f1/f2/l3a");
  MAKE_PATH("/f1/f2/l3b");

  char walk_res[200];
  char* out = walk_res;
  walk("/", &out);

  // Blank line for spacing purposes
  log_event("");
  printf("%s", walk_res);
}

void nested_files(void) {
  const char* top_file_path = "/foo";
  int top_file = open(top_file_path, O_WRONLY);
  assert(top_file != -1);

  const char* folder_file_1_path = "/abc/a";
  int folder_file_1 = open(folder_file_1_path, O_WRONLY);
  assert(folder_file_1 != -1);

  const char* folder_file_2_path = "/abc/b";
  int folder_file_2 = open(folder_file_2_path, O_WRONLY);
  assert(folder_file_2 != -1);

  close(top_file);
  close(folder_file_1);
  close(folder_file_2);

  // Delete second file in folder
  assert(!remove(folder_file_2_path));
  assert(open(folder_file_2_path, O_RDONLY) == -1);
  // First is still present
  int test_handle = open(folder_file_1_path, O_RDONLY);
  assert(test_handle != -1);
  close(test_handle);

  // Now remove the folder
  assert(!remove("/abc"));
  // Check that the first folder file is gone
  // (at least unreachable)
  assert(open(folder_file_1_path, O_RDONLY) == -1);
}

#define FS_TEST(test)                                                          \
  void test_##test() {                                                         \
    init_file_system();                                                        \
    test();                                                                    \
    destroy_file_system();                                                     \
  }
FS_TEST(errors)
FS_TEST(basics)
FS_TEST(nested_files)
FS_TEST(list_files)

void setup() {
  KernelConfig cfg = { .log_scheduler=false,
                       .destroy_on_stack_err=false};
  k_set_kernel_config(&cfg);

#define ADD_TEST(test) k_add_named_thread(test_##test, #test)
  ADD_TEST(errors);
  ADD_TEST(basics);
  ADD_TEST(nested_files);
  ADD_TEST(list_files);
}

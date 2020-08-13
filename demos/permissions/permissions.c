#include "user/thread.h"
#include "thread.h"
#include "util.h"
#include "file.h"
#include "user/file.h"
#include "user/alloc.h"

void cannot_alloc() {
  assert(malloc(sizeof(int)) == NULL);
  // Some random address so we don't just go straight to k_malloc()
  assert(realloc((void*)0xcafef00d, sizeof(int)) == NULL);
  // Doesn't prove much, except that we don't crash
  free(NULL);
}

int read_fd;
int write_fd;

void cannot_file() {
  // Some file that should always be there
  assert(open("README.md", O_RDONLY) == -1);
  assert(list_dir(".", NULL, 0) == -1);
  // nbyte must be non zero so that zero isn't
  // an expected return value
  char* tmp = "hello";
  assert(write(write_fd, tmp, 5) == 0);
  assert(read(read_fd, tmp, 5) == 0);
  assert(lseek(read_fd, 33, SEEK_CUR) == -1);
  assert(remove("permissions") == -1);
  assert(close(read_fd) == -1);
}

void cannot_create() {
  assert(add_thread_from_worker(cannot_create) == -1);
}

void cannot_kconfig() {
  // Note: we allow reads
  // We'll know if this was allowed by the log output
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);
}

void cannot_tconfig() {
  assert(!set_thread_name(-1, "won't see this!"));
}

void cannot_mutli() {
  // Grab back just to check we handle mutiple
  // permissions being removed
  assert(close(read_fd) == -1);
  assert(malloc(sizeof(int)) == NULL);
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);
  // This one we are allowed to do
  assert(set_thread_name(-1, "new_name"));
}

void cleanup() {
  assert(close(read_fd) == 0);
  assert(close(write_fd) == 0);
}

// Use this to run one by one so we aren't limited
// by MAX_THREADS
void runner() {
  int tid = add_thread("noalloc", NULL, cannot_alloc,
    THREAD_FUNC | TPERM_NO_ALLOC);
  assert(tid != -1);
  set_child(tid);
  yield();

  // Since cannot_file obviously can't make these
  read_fd = open("CMakeLists.txt", O_RDONLY);
  write_fd = open("__perm_demo", O_WRONLY);

  tid = add_thread("nofile", NULL, cannot_file,
    THREAD_FUNC | TPERM_NO_FILE);
  set_child(tid);
  yield();

  tid = add_thread("nocreate", NULL, cannot_create,
    THREAD_FUNC | TPERM_NO_CREATE);
  set_child(tid);
  yield();

  tid = add_thread("nokconfig", NULL, cannot_kconfig,
    THREAD_FUNC | TPERM_NO_KCONFIG);
  set_child(tid);
  yield();

  tid = add_thread("notconfig", NULL, cannot_tconfig,
    THREAD_FUNC | TPERM_NO_TCONFIG);
  set_child(tid);
  yield();

  tid = add_thread("nomulti", NULL, cannot_mutli,
    THREAD_FUNC | TPERM_NO_FILE | TPERM_NO_ALLOC |
    TPERM_NO_KCONFIG);
  set_child(tid);
  yield();

  tid = add_thread("cleanup", NULL, cleanup,
    THREAD_FUNC);
  set_child(tid);
  yield();
}

void setup(void) {
  // This will inherit kernel permissions (everything)
  assert(k_add_thread("runner", NULL, runner,
    THREAD_FUNC) != -1);
}

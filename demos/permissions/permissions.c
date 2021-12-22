#include "common/assert.h"
#include "user/alloc.h"
#include "user/errno.h"
#include "user/file.h"
#include "user/thread.h"

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
  errno = 0;
  assert(open(SRC_ROOT "/README.md", O_RDONLY) == -1);
  assert(errno == E_PERM);
  errno = 0;
  assert(list_dir(".", NULL, 0) == -1);
  assert(errno == E_PERM);
  // nbyte must be non zero so that zero isn't
  // an expected return value
  char* tmp = "hello";
  errno = 0;
  assert(write(write_fd, tmp, 5) == 0);
  assert(errno == E_PERM);
  errno = 0;
  assert(read(read_fd, tmp, 5) == 0);
  assert(errno == E_PERM);
  errno = 0;
  assert(lseek(read_fd, 33, SEEK_CUR) == -1);
  assert(errno == E_PERM);
  errno = 0;
  assert(remove("permissions") == -1);
  assert(errno == E_PERM);
  errno = 0;
  assert(close(read_fd) == -1);
  assert(errno == E_PERM);
}

void cannot_create() {
  assert(add_thread_from_worker(cannot_create) == INVALID_THREAD);
}

void cannot_kconfig() {
  // Note: we allow reads
  // We'll know if this was allowed by the log output
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);
}

void cannot_tconfig() {
  // Can't config self
  assert(!set_thread_name(CURRENT_THREAD, "won't see this!"));
  // Can config other
  assert(set_thread_name(0, "runner"));
}

void cannot_tconfig_other() {
  // Can't config other
  assert(!set_thread_name(0, "won't see this either!"));
  // Can config self
  assert(set_thread_name(CURRENT_THREAD, "no_tconfig_other"));
}

void cannot_mutli() {
  // Grab back just to check we handle multiple
  // permissions being removed
  assert(close(read_fd) == -1);
  assert(malloc(sizeof(int)) == NULL);
  set_kernel_config(KCFG_LOG_SCHEDULER, 0);
  // This one we are allowed to do
  assert(set_thread_name(CURRENT_THREAD, "new_name"));
}

void same_permissions() {
  errno = 0;
  assert(set_thread_name(CURRENT_THREAD, "SAME"));
  assert(errno == 0);
}

void reduced_permissions() {
  errno = 0;
  assert(!set_thread_name(CURRENT_THREAD, "REDUCED"));
  assert(errno == E_PERM);
}

// Check that threads created by user threads also inherit
void user_inherit() {
  // Check that errno is per thread
  errno = 0;

  // This thread will have same permissions
  ThreadFlags flags = {.is_file = false, .remove_permissions = 0};
  int tid = add_thread("same", NULL, same_permissions, &flags);
  set_child(tid);
  yield();

  // This has one removed
  flags.remove_permissions = TPERM_TCONFIG;
  tid = add_thread("reduced", NULL, reduced_permissions, &flags);

  set_child(tid);
  yield();

  // Would be E_PERM if it weren't per thread
  assert(errno == 0);
}

void user_reduce() {
  // Same idea as the others except we remove
  // permissons within the same thread.
  uint16_t perm = permissions(TPERM_FILE);
  assert((perm & TPERM_FILE) == 0);
  cannot_file();

  // ... assume the rest follow the same pattern
  // Except for TCONFIG, which means we can't change
  // our permissions anymore. (which makes sense but
  // was surprising to me at least)
  perm = permissions(TPERM_TCONFIG);
  cannot_tconfig();

  assert(perm & TPERM_KCONFIG);
  perm = permissions(TPERM_KCONFIG);
  assert(perm & TPERM_KCONFIG);
}

void errno_checks() {
  // Check non E_PERM values
  errno = 0;
  assert(!set_thread_name(-99, "food"));
  assert(errno == E_INVALID_ID);
  errno = 0;
  char tname[THREAD_NAME_SIZE];
  assert(!thread_name(-99, tname));
  assert(errno == E_INVALID_ID);
}

void cleanup() {
  assert(close(read_fd) == 0);
  assert(close(write_fd) == 0);
}

#define RUN_TEST_THREAD(NAME, FN, FLAGS)                                       \
  flags.remove_permissions = FLAGS;                                            \
  tid = add_thread(NAME, NULL, FN, &flags);                                    \
  assert(tid != INVALID_THREAD);                                               \
  set_child(tid);                                                              \
  yield();

#define CHECK_TPROP_FAILS(fn, property, ptr)                                   \
  errno = 0;                                                                   \
  assert(!fn(CURRENT_THREAD, property, ptr));                                  \
  assert(errno == E_INVALID_ARGS);

// Use this to run one by one so we aren't limited
// by MAX_THREADS
void setup(void) {
  int tid = INVALID_THREAD;

  // Random errno checks we have no better place for
  // NULL value/dest ptrs rejected
  CHECK_TPROP_FAILS(get_thread_property, TPROP_REGISTERS, NULL);
  CHECK_TPROP_FAILS(get_thread_property, TPROP_REGISTERS, NULL);
  // Invalid properties rejected
  CHECK_TPROP_FAILS(set_thread_property, -1, (void*)(1));
  CHECK_TPROP_FAILS(get_thread_property, -1, (void*)(1));

  set_thread_name(CURRENT_THREAD, "runner");
  ThreadFlags flags;
  flags.is_file = false;

  RUN_TEST_THREAD("noalloc", cannot_alloc, TPERM_ALLOC);

  // Since cannot_file obviously can't make these
  read_fd = open(SRC_ROOT "/CMakeLists.txt", O_RDONLY);
  assert(read_fd != -1);
  write_fd = open("__perm_demo", O_WRONLY);
  assert(write_fd != -1);

  RUN_TEST_THREAD("nofile", cannot_file, TPERM_FILE);
  RUN_TEST_THREAD("nocreate", cannot_create, TPERM_CREATE);
  RUN_TEST_THREAD("nokconfig", cannot_kconfig, TPERM_KCONFIG);
  RUN_TEST_THREAD("notconfig", cannot_tconfig, TPERM_TCONFIG);
  RUN_TEST_THREAD("notconfigother", cannot_tconfig_other, TPERM_TCONFIG_OTHER);
  RUN_TEST_THREAD("nomulti", cannot_mutli,
                  TPERM_FILE | TPERM_ALLOC | TPERM_KCONFIG);
  RUN_TEST_THREAD("userinherit", user_inherit, 0);
  RUN_TEST_THREAD("userreduce", user_reduce, 0);
  RUN_TEST_THREAD("errno", errno_checks, 0);

  RUN_TEST_THREAD("cleanup", cleanup, 0);
}

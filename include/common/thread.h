#ifndef COMMON_THREAD_H
#define COMMON_THREAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Thread related stuff available to user and kernel

#define INVALID_THREAD -1
// For the set/get property API
#define CURRENT_THREAD INVALID_THREAD

// Including null terminator
// So use this as the size of any buffer to hold a thread name
#define THREAD_NAME_SIZE 13
// Max strlen() of a thread name
#define THREAD_NAME_MAX_LEN (THREAD_NAME_SIZE - 1)

/* clang-format off */

#define KCFG_DESTROY_ON_STACK_ERR 1 << 0 // Destroy threads with stack issues
                                         // (instead of exiting the kernel completely)
#define KCFG_LOG_SCHEDULER        1 << 1 // Print messages from the scheduler
#define KCFG_LOG_THREADS          1 << 2 // Print messages from threads
#define KCFG_COLOUR_OUTPUT        1 << 3 // Apply colours to messages
#define KCFG_LOG_FAILED_ERRNO     1 << 4 // Log failed syscalls

/* [[[cog
import cog
from scripts.thread_properties import properties
name_len = max([len(d) for _,_,d,_ in properties])
for number, name, define, type in properties:
  cog.outl("#define TPROP_{} {}".format(define.ljust(name_len), number))
]]] */
#define TPROP_NAME            0
#define TPROP_CHILD           1
#define TPROP_PARENT          2
#define TPROP_STATE           3
#define TPROP_PERMISSIONS     4
#define TPROP_REGISTERS       5
#define TPROP_PENDING_SIGNALS 6
#define TPROP_SIGNAL_HANDLER  7
/* [[[end]]] */

// Passed to the add_thread syscall
typedef struct {
  bool is_file;
  uint16_t remove_permissions;
} ThreadFlags;

#define TPERM_NONE          (0)
#define TPERM_ALL           (0xFFFF)
#define TPERM_CREATE        (1 << 0) // Create new threads
#define TPERM_FILE          (1 << 1) // Read/write files
#define TPERM_ALLOC         (1 << 2) // Dynamically allocate memory
#define TPERM_KCONFIG       (1 << 3) // Change the kernel configuration
#define TPERM_TCONFIG       (1 << 4) // Configure the current thread
#define TPERM_TCONFIG_OTHER (1 << 5) // Configure other threads

/* clang-format on */

#define YIELD_ANY 0
#define YIELD_TO  1

typedef enum {
  init = 0,
  running = 1,
  suspended = 2,
  waiting = 3,
  finished = 4,
  cancelled = 5,
} ThreadState;

typedef struct {
  size_t a1;
  size_t a2;
  size_t a3;
  size_t a4;
} ThreadArgs;

// Note the () around the args so we don't mess up pointer
// arithmetic e.g. make_args(argv+1...) => (size_t)(argv+1)
#define make_args(a, b, c, d)                                                  \
  { (size_t)(a), (size_t)(b), (size_t)(c), (size_t)(d) }

#ifdef KERNEL
#define USER_CONST
#else
#define USER_CONST const
#endif

// This structure is available to threads from userspace
// and allows us to avoid syscalls for common operations.
typedef struct {
  USER_CONST int id;
  USER_CONST char name[THREAD_NAME_SIZE];
  USER_CONST uint32_t kernel_config;
  int err_no;
} UserThreadInfo;
extern UserThreadInfo user_thread_info;

#endif /* ifdef COMMON_THREAD_H */

#include "kernel/thread.h"
#include "common/assert.h"
#include "common/errno.h"
#include "common/print.h"
#include "common/trace.h"
#include "port/port.h"
// So we can user log_event on exit
#include "user/thread.h"
#if CODE_PAGE_SIZE
#include "kernel/elf.h"
#endif
#include "kernel/alloc.h"
#include "kernel/file.h"
#include <stdarg.h>
#include <string.h>

__attribute__((section(".thread_vars_bss"))) Thread* current_thread;

__attribute__((section(".thread_structs"))) Thread all_threads[MAX_THREADS];
__attribute__((section(".thread_vars_bss"))) Thread* next_thread;

__attribute__((section(".thread_vars"))) uint32_t kernel_config =
    KCFG_LOG_THREADS;

#if CODE_PAGE_SIZE
__attribute__((section(".code_page"))) uint8_t code_page[CODE_PAGE_SIZE];
#if CODE_BACKING_PAGES
#define INVALID_PAGE 123456
__attribute__((section(".code_page_backing")))
uint8_t code_page_backing[CODE_BACKING_PAGES][CODE_PAGE_SIZE];
#endif
#endif /* CODE_PAGE_SIZE */

extern void setup(void);
extern void load_first_thread(void);
void check_stack(void);

bool k_has_no_permission(uint16_t permission) {
  if (!current_thread) {
    // Must be kernel (famous last words)
    return false;
  }
  uint16_t has = current_thread->permissions;
  return (has & permission) == 0;
}

void k_set_kernel_config(uint32_t enable, uint32_t disable) {
  if (k_has_no_permission(TPERM_KCONFIG)) {
    return;
  }

  kernel_config |= enable;
  kernel_config &= ~disable;
}

uint32_t k_get_kernel_config(void) {
  return kernel_config;
}

bool is_valid_thread(int tid) {
  return (tid >= 0) && (tid < MAX_THREADS) &&
         all_threads[tid].id != INVALID_THREAD;
}

static bool can_schedule_thread(int tid) {
  return is_valid_thread(tid) &&
         (all_threads[tid].state == suspended ||
          all_threads[tid].state == init || all_threads[tid].state == running);
}

int k_get_thread_id(void) {
  return current_thread ? current_thread->id : INVALID_THREAD;
}

static void k_set_thread_name(Thread* thread, const char* name) {
  if (name) {
    // Will null pad if name is smaller than buffer
    strncpy(thread->name, name, THREAD_NAME_MAX_LEN);
    thread->name[THREAD_NAME_MAX_LEN] = '\0';
  } else {
    thread->name[0] = '\0';
  }
}

bool k_set_thread_property(int tid, size_t property, const void* value) {
  if (((tid == CURRENT_THREAD) && k_has_no_permission(TPERM_TCONFIG)) ||
      ((tid != CURRENT_THREAD) && k_has_no_permission(TPERM_TCONFIG_OTHER))) {
    current_thread->err_no = E_PERM;
    return false;
  }

  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    current_thread->err_no = E_INVALID_ID;
    return false;
  }

  Thread* thread = &all_threads[tid];
  switch (property) {
    case TPROP_CHILD: {
      // Not sure I like one property call setting two things
      int child = *(const int*)value;
      if (is_valid_thread(child)) {
        thread->child = child;
        all_threads[child].parent = tid;
      }
      break;
    }
    case TPROP_NAME:
      k_set_thread_name(thread, *(const char**)value);
      break;
    case TPROP_PERMISSIONS:
      thread->permissions &= ~(*(uint16_t*)value);
      break;
    case TPROP_REGISTERS: {
      // Note that setting registers on an init thread doesn't
      // serve much purpose but it won't break anything.
      RegisterContext* ctx = (RegisterContext*)value;
      memcpy(thread->stack_ptr, ctx, sizeof(RegisterContext));
      break;
    }
    case TPROP_PENDING_SIGNALS: {
      uint32_t signal = *(uint32_t*)value;
      if (signal) {
        thread->pending_signals |= 1 << (signal - 1);
      }
      break;
    }
    case TPROP_SIGNAL_HANDLER:
      thread->signal_handler = *(void (**)(uint32_t))value;
      break;
    default:
      assert(0);
  }

  return true;
}

bool k_get_thread_property(int tid, size_t property, void* res) {
  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    current_thread->err_no = E_INVALID_ID;
    return false;
  }

  Thread* thread = &all_threads[tid];
  switch (property) {
    // Cast to correct type is important here so that
    // we don't overwrite more of the result than expected.
    case TPROP_ID:
      *(int*)res = thread->id;
      break;
    case TPROP_NAME: {
      char* dest = (char*)res;
      if (dest) {
        strncpy(dest, thread->name, THREAD_NAME_MAX_LEN);
        dest[strlen(thread->name)] = '\0';
      }
      break;
    }
    case TPROP_CHILD:
      *(int*)res = thread->child;
      break;
    case TPROP_STATE:
      // This one is a bit weird given that it's
      // actually a size_t in the struct.
      *(ThreadState*)res = thread->state;
      break;
    case TPROP_PERMISSIONS:
      *(uint16_t*)res = thread->permissions;
      break;
    case TPROP_REGISTERS: {
      RegisterContext* ctx = (RegisterContext*)res;
      *ctx = *(RegisterContext*)thread->stack_ptr;
      break;
    }
    case TPROP_ERRNO_PTR:
      *(int**)res = &current_thread->err_no;
      break;
    default:
      assert(0);
  }

  return true;
}

void init_thread(Thread* thread, int tid, const char* name,
                 void (*do_work)(void), const ThreadArgs* args,
                 uint16_t permissions) {
  // thread_start will jump to this
  thread->work = do_work;
  thread->signal_handler = NULL;
  thread->state = init;
  thread->pending_signals = 0;
  thread->id = tid;
  thread->args = *args;
  thread->parent = INVALID_THREAD;
  thread->child = INVALID_THREAD;
  thread->permissions = permissions;
  thread->err_no = 0;

  k_set_thread_name(thread, name);

  // Start message buffer empty
  thread->next_msg = &(thread->messages[0]);
  thread->end_msgs = thread->next_msg;
  thread->msgs_full = false;

#if CODE_PAGE_SIZE
  thread->in_code_page = false;
#if CODE_BACKING_PAGES
  thread->code_backing_page = INVALID_PAGE;
#endif
#endif

  thread->bottom_canary = STACK_CANARY;
  thread->top_canary = STACK_CANARY;
  // Top of stack
  size_t stack_ptr = (size_t)(&(thread->stack[THREAD_STACK_SIZE]));
  // Mask to align to 16 bytes for AArch64
  thread->stack_ptr = (uint8_t*)(ALIGN_STACK_PTR(stack_ptr));

  // Setup the initial restore frame
  init_register_context(thread);
}

__attribute__((constructor)) static void init_threads(void) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    ThreadArgs noargs = {0, 0, 0, 0};
    init_thread(&all_threads[idx], INVALID_THREAD, NULL, NULL, &noargs,
                TPERM_NONE);
  }
}

static int k_add_named_thread_with_args(void (*worker)(), const char* name,
                                        const ThreadArgs* args,
                                        uint16_t remove_permissions);
__attribute__((noreturn)) void entry(void) {
  extern char _etext, _data, _edata, _bstart, _bend;

  // Copy .data sections
  memcpy(&_data, &_etext, &_edata - &_data);

  // Zero bss
  memset(&_bstart, 0, &_bend - &_bstart);

  // Call constructors
  typedef void (*fn_ptr)(void);
  extern fn_ptr _init_array, _einit_array;
  for (fn_ptr* fn = &_init_array; fn < &_einit_array; ++fn) {
    (*fn)();
  }

  if (k_stdout_isatty()) {
    kernel_config |= KCFG_COLOUR_OUTPUT;
  }

  ThreadArgs empty_args = make_args(0, 0, 0, 0);

#if defined CODE_PAGE_SIZE && defined STARTUP_PROG
  const char* startup_prog = STARTUP_PROG;
  k_log_event("Loading program \"%s\"", startup_prog);
  // Empty means load builtin threads
  if (strcmp(startup_prog, "")) {
    int tid = k_add_thread_from_file_with_args(startup_prog, &empty_args, 0);
    if (tid == INVALID_THREAD) {
      // No errno for kernel, so some generic phrasing
      k_log_event("Failed to load STARTUP_PROG \"%s\"", startup_prog);
      k_exit(1);
    }
  } else {
#else
  {
#endif
    k_add_named_thread_with_args(setup, NULL, &empty_args, 0);
  }

  load_first_thread();

  __builtin_unreachable();
}

void k_log_event(const char* event, ...) {
  if (!(kernel_config & KCFG_LOG_THREADS)) {
    return;
  }

  bool colour = kernel_config & KCFG_COLOUR_OUTPUT;
  char thread_name[THREAD_NAME_SIZE];
  const char* name = NULL;
  if (current_thread) {
    name = current_thread->name;
  }
  format_thread_name(thread_name, k_get_thread_id(), name);
  printf("%sThread %s:%s ", colour ? text_colour(eYellow) : "", thread_name,
         colour ? text_colour(eReset) : "");

  va_list args;
  va_start(args, event);
  vprintf(event, args);
  va_end(args);

  printf("\n");
}

void log_scheduler_event(const char* event) {
  if (kernel_config & KCFG_LOG_SCHEDULER) {
    bool colour = kernel_config & KCFG_COLOUR_OUTPUT;
    printf("%sThread  <scheduler>:%s %s\n", colour ? text_colour(eYellow) : "",
           colour ? text_colour(eReset) : "", event);
  }
}

#if CODE_BACKING_PAGES
static void swap_paged_threads(const Thread* current, const Thread* next) {
  if (current == next) {
    return;
  }

  // See if we need to swap out current thread
  if (current && (current->code_backing_page != INVALID_PAGE)) {
    memcpy(code_page_backing[current->code_backing_page], code_page,
           CODE_PAGE_SIZE);
  }
  // If the next thread's code is in a backing page
  if (next->code_backing_page != INVALID_PAGE) {
    memcpy(code_page, code_page_backing[next->code_backing_page],
           CODE_PAGE_SIZE);
  }
}
#endif

static size_t next_possible_thread_idx(const Thread* curr) {
  if (curr) {
    // Parent always yields to child if possible
    if (is_valid_thread(curr->child)) {
      return curr->child;
    }

    // Children always return to parents
    if (is_valid_thread(curr->parent)) {
      return curr->parent;
    }

    // +1 to skip the current thread
    return curr - &all_threads[0] + 1;
  }

  // On startup current_thread will be NULL
  return 0;
}

// Basic scheduler that walks the all_threads array
static Thread* choose_next_thread(void) {
  size_t start_thread_idx = next_possible_thread_idx(current_thread);

  size_t max_thread_idx = start_thread_idx + MAX_THREADS;
  for (size_t idx = start_thread_idx; idx < max_thread_idx; ++idx) {
    // Wrap into range of array
    size_t _idx = idx % MAX_THREADS;

    if (!can_schedule_thread(_idx)) {
      continue;
    }

    // We know that the ID is not -1 at this point
    if ((size_t)all_threads[_idx].id != _idx) {
      printf("thread ID %u and position %u inconsistent!\n", (unsigned)_idx,
             all_threads[_idx].id);
      k_exit(1);
    }

    log_scheduler_event("scheduling new thread");

    log_scheduler_event("next thread chosen");
    return &all_threads[_idx];
  }
  return NULL;
}

void do_scheduler(void) {
  // NULL means the scheduler should pick the next thread
  if (next_thread == NULL) {
    next_thread = choose_next_thread();
  }

  // If we couldn't find a thread to switch to
  if (next_thread == NULL) {
    // If the current thread is the last one, just return to it
    if (can_schedule_thread(k_get_thread_id())) {
      next_thread = current_thread;
    } else {
      // Otherwise exit the kernel completely
      log_scheduler_event("all threads finished");
      k_exit(0);
    }
  }

  // Don't change cancelled/waiting etc.
  if (current_thread && current_thread->state == running) {
    current_thread->state = suspended;
  }
  next_thread->state = running;

#if CODE_BACKING_PAGES
  swap_paged_threads(current_thread, next_thread);
#endif
  check_signals(next_thread);
}

static bool set_thread_state(int tid, ThreadState state) {
  if (is_valid_thread(tid)) {
    all_threads[tid].state = state;
    return true;
  }
  return false;
}

bool k_thread_wake(int tid) {
  return set_thread_state(tid, suspended);
}

static void cleanup_thread(Thread* thread) {
  // Free any lingering heap allocations
  k_free_all(thread->id);

#if CODE_PAGE_SIZE
  // Free the code page. If there are other threads
  // running from it they will still have true to keep it alive.
  thread->in_code_page = false;
#if CODE_BACKING_PAGES
  thread->code_backing_page = INVALID_PAGE;
#endif
#endif
}

bool k_thread_cancel(int tid) {
  if (tid == CURRENT_THREAD) {
    tid = current_thread->id;
  }

  bool set = set_thread_state(tid, cancelled);
  if (set) {
    cleanup_thread(&all_threads[tid]);

    if (tid == current_thread->id) {
      // Thread cancelled itself, choose another
      do_scheduler();
      return true;
    }
  }
  return set;
}

static bool k_do_yield(Thread* to) {
  check_stack();

  next_thread = to;
  // Assembly handler will see next_thread set and do the switch
  do_scheduler();
  // Return true if we found a new thread to yield to
  return next_thread && (next_thread != current_thread);
}

bool k_yield(int tid, int kind) {
  switch (kind) {
    case YIELD_ANY:
      assert(tid == INVALID_THREAD);
      return k_do_yield(NULL);
    case YIELD_TO:
      if (!can_schedule_thread(tid)) {
        return false;
      }
      return k_do_yield(&all_threads[tid]);
    default:
      assert(0);
      break;
  }

  __builtin_unreachable();
}

#if CODE_PAGE_SIZE && !CODE_BACKING_PAGES
static int code_page_in_use_by() {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    if (all_threads[idx].in_code_page) {
      return all_threads[idx].id;
    }
  }
  return INVALID_THREAD;
}
#endif

#if CODE_BACKING_PAGES
static size_t find_free_backing_page(void) {
  bool possible[CODE_BACKING_PAGES];
  for (size_t i = 0; i < CODE_BACKING_PAGES; ++i) {
    possible[i] = true;
  }
  for (size_t i = 0; i < MAX_THREADS; ++i) {
    size_t page = all_threads[i].code_backing_page;
    if (page != INVALID_PAGE) {
      possible[page] = false;
    }
  }
  for (size_t i = 0; i < CODE_BACKING_PAGES; ++i) {
    if (possible[i]) {
      return i;
    }
  }
  return INVALID_PAGE;
}
#endif

#if CODE_PAGE_SIZE
static void setup_code_page(size_t idx) {
  Thread* curr = current_thread;
#if CODE_BACKING_PAGES
  if (curr) {
    size_t page = curr->code_backing_page;
    if (page != INVALID_PAGE) {
      all_threads[idx].code_backing_page = page;
    }
  }
#else
  // Check null because we might be in setup() which runs as kernel
  if (curr && curr->in_code_page) {
    // Code page must live as long as all threads created by code in it
    all_threads[idx].in_code_page = true;
  }
#endif /* CODE_BACKING_PAGES */
}
#endif /* CODE_PAGE_SIZE */

static int k_add_named_thread_with_args(void (*worker)(), const char* name,
                                        const ThreadArgs* args,
                                        uint16_t remove_permissions) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    if (all_threads[idx].id == INVALID_THREAD ||
        all_threads[idx].state == cancelled ||
        all_threads[idx].state == finished) {

      // New thread's permissions is the current thread's
      // minus any it wants to remove
      uint16_t permissions = TPERM_ALL;
      if (current_thread) {
        permissions = current_thread->permissions;
      }
      permissions &= ~remove_permissions;

      init_thread(&all_threads[idx], idx, name, worker, args, permissions);
#if CODE_PAGE_SIZE
      setup_code_page(idx);
#endif
      return idx;
    }
  }
  return INVALID_THREAD;
}

#if CODE_PAGE_SIZE
int k_add_thread_from_file_with_args(const char* filename,
                                     const ThreadArgs* args,
                                     uint16_t remove_permissions) {
#if CODE_BACKING_PAGES
  size_t free_page = find_free_backing_page();
  // If we have backing, don't count the active
  // page as useable for loading.
  if (free_page == INVALID_PAGE) {
#else
  if (code_page_in_use_by() != INVALID_THREAD) {
#endif
    if (current_thread) {
      current_thread->err_no = E_NO_PAGE;
    }
    return INVALID_THREAD;
  }

  uint8_t* dest = code_page;
#if CODE_BACKING_PAGES
  dest = code_page_backing[free_page];
#endif

  void (*entry)() = load_elf(filename, dest);
  if (!entry) {
    // Startup may call this to load STARTUP_PROG
    if (current_thread) {
      current_thread->err_no = E_NOT_FOUND;
    }
    return INVALID_THREAD;
  }
  int tid =
      k_add_named_thread_with_args(entry, filename, args, remove_permissions);

#if CODE_BACKING_PAGES
  all_threads[tid].code_backing_page = free_page;
#else
  all_threads[tid].in_code_page = true;
#endif
  return tid;
}
#endif

int k_add_thread(const char* name, const ThreadArgs* args, void* worker,
                 uint32_t flags) {
  if (k_has_no_permission(TPERM_CREATE)) {
    return INVALID_THREAD;
  }

  ThreadArgs dummy_args = {0, 0, 0, 0};
  if (!args) {
    args = &dummy_args;
  }

  uint16_t kind = flags & TFLAG_KIND_MASK;
  uint16_t remove_permissions = flags >> TFLAG_PERM_SHIFT;
  switch (kind) {
    case THREAD_FILE:
#ifndef CODE_PAGE_SIZE
      assert(0);
#else
      return k_add_thread_from_file_with_args((const char*)worker, args,
                                              remove_permissions);
#endif
    case THREAD_FUNC:
      return k_add_named_thread_with_args(worker, name, args,
                                          remove_permissions);
    default:
      assert(0 && "invalid flags!");
      break;
  }

  __builtin_unreachable();
}

void k_thread_wait(void) {
  current_thread->state = waiting;
  // Manually move to next thread
  do_scheduler();
}

// Use these struct names to ensure that these are
// placed *after* the thread structs to prevent
// stack overflow corrupting them.
__attribute__((section(".thread_vars"))) size_t thread_stack_offset =
    offsetof(Thread, stack);

extern void load_next_thread(void);
void check_stack(void) {
  bool underflow = current_thread->bottom_canary != STACK_CANARY;
  bool overflow = current_thread->top_canary != STACK_CANARY;

  if (underflow || overflow) {
    // Don't schedule this again, or rely on its ID
    current_thread->id = INVALID_THREAD;
    current_thread->name[0] = '\0';

    if (underflow) {
      k_log_event("Stack underflow!");
    }
    if (overflow) {
      k_log_event("Stack overflow!");
    }

    if (kernel_config & KCFG_DESTROY_ON_STACK_ERR) {
      /* Setting INVALID_THREAD here, instead of state=finished is fine,
          because A: the thread didn't actually finish
                  B: the thread struct is actually invalid */
      current_thread->id = INVALID_THREAD;

      // Would clear heap allocs here but we can't trust the thread ID

      // Aka don't save any state, just load the scheduler
      // TODO: this isn't actually checked by the assembly
      current_thread->state = init;
      // TODO: we're probably losing kernel stack space every time we do this.
      // (on arches with banked stack pointers)
      load_next_thread();
    } else {
      // TODO: not covered by tests
      k_exit(1);
    }
  }
}

__attribute__((noreturn)) void thread_start(void) {
  // Every thread starts by entering this function

  // Call thread's actual function
  current_thread->work(current_thread->args.a1, current_thread->args.a2,
                       current_thread->args.a3, current_thread->args.a4);

  cleanup_thread(current_thread);
  log_event("exiting");
  // Set this last so if we're interrupted after this
  // it's not an issue.
  current_thread->state = finished;

  // Yielding directly so we don't print "yielding"
  YIELD_ASM;

  __builtin_unreachable();
}

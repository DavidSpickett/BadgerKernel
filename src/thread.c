#include "print.h"
#include "thread.h"
#include "common/trace.h"
#include "common/errno.h"
#include "util.h"
#if CODE_PAGE_SIZE
#include "elf.h"
#endif
#include <string.h>
#include <stdarg.h>
#include "alloc.h"

__attribute__((section(".thread_vars"))) Thread* _current_thread;

__attribute__((section(".thread_structs"))) Thread all_threads[MAX_THREADS];
__attribute__((section(".thread_vars"))) Thread* next_thread;

__attribute__((section(".thread_vars")))
uint32_t kernel_config = KCFG_LOG_THREADS;

#if CODE_PAGE_SIZE
__attribute__((section(".code_page"))) uint8_t code_page[CODE_PAGE_SIZE];
#if CODE_BACKING_PAGES
#define INVALID_PAGE 123456
__attribute__((section(".code_page_backing")))
uint8_t code_page_backing[CODE_BACKING_PAGES][CODE_PAGE_SIZE];
#endif
#endif /* CODE_PAGE_SIZE */

bool k_has_no_permission(uint16_t permission) {
  if (!_current_thread) {
    // Must be kernel (famous last words)
    return false;
  }
  uint16_t has = _current_thread->permissions;
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
  return is_valid_thread(tid) && \
    (all_threads[tid].state == suspended ||
     all_threads[tid].state == init ||
     all_threads[tid].state == running);
}

Thread* current_thread(void);

void k_invalid_syscall(size_t arg1, size_t arg2, size_t arg3, size_t arg4) {
  printf("Unknown syscall invoked!\n");
  printf("arg1: %u, arg2: %u, arg3: %u, arg4: %u\n",
    arg1, arg2, arg3, arg4);
  k_exit(1);
}

int k_get_thread_id(void) {
  return _current_thread ? _current_thread->id : INVALID_THREAD;
}

bool k_set_thread_property(int tid, size_t property,
                           const void* value) {
  if (
    ((tid == CURRENT_THREAD) &&
      k_has_no_permission(TPERM_TCONFIG)) ||
    ((tid != CURRENT_THREAD) &&
      k_has_no_permission(TPERM_TCONFIG_OTHER))
  ) {
    _current_thread->err_no = E_PERM;
    return false;
  }

  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    _current_thread->err_no = E_INVALID_ID;
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
      thread->name = *(const char**)value;
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
        thread->pending_signals |=  1 << (signal-1);
      }
      break;
    }
    case TPROP_SIGNAL_HANDLER:
      thread->signal_handler = *(void (**)(uint32_t))value;
      break;
    default:
      assert(0);
      return false;
  }

  return true;
}


bool k_get_thread_property(int tid, size_t property,
                           void* res) {
  if (tid == CURRENT_THREAD) {
    tid = k_get_thread_id();
  }
  if (!is_valid_thread(tid)) {
    _current_thread->err_no = E_INVALID_ID;
    return false;
  }

  Thread* thread = &all_threads[tid];
  switch (property) {
    // Cast to correct type is important here so that
    // we don't overwrite more of the result than expected.
    case TPROP_ID:
      *(int*)res = thread->id;
      break;
    case TPROP_NAME:
      *(const char**)res = thread->name;
      break;
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
      *(int**)res = &_current_thread->err_no;
      break;
    default:
      assert(0);
      return false;
  }

  return true;
}

void init_thread(Thread* thread, int tid, const char* name,
                 void (*do_work)(void), const ThreadArgs* args,
                 uint16_t permissions)
{
  // thread_start will jump to this
  thread->work = do_work;
  thread->signal_handler = NULL;
  thread->state = init;
  thread->pending_signals = 0;

  thread->id = tid;
  thread->name = name;
  thread->args = *args;
  thread->parent = INVALID_THREAD;
  thread->child = INVALID_THREAD;
  thread->permissions = permissions;
  thread->err_no = 0;

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
  thread->stack_ptr = (uint8_t*)(stack_ptr & ~0xF);

  // Setup the initial restore frame
  init_register_context(thread);
}

extern void setup(void);
extern void load_first_thread(void);
// TODO: collect all these forward declarations
static int k_add_named_thread_with_args(void (*worker)(), const char* name,
                                 const ThreadArgs* args, uint16_t remove_permissions);
__attribute__((noreturn)) void entry(void) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    ThreadArgs noargs = {0, 0, 0, 0};
    init_thread(&all_threads[idx], INVALID_THREAD,
      NULL, NULL, &noargs, TPERM_NONE);
  }

  ThreadArgs empty_args = make_args(0,0,0,0);

#if defined CODE_PAGE_SIZE && defined STARTUP_PROG
  const char* startup_prog = STARTUP_PROG;
  k_log_event("Loading program \"%s\"", startup_prog);
  // Empty means load builtin threads
  if (strcmp(startup_prog, "")) {
    int tid = k_add_thread_from_file_with_args(
      startup_prog, &empty_args, 0);
    if (tid == INVALID_THREAD) {
      // No errno for kernel, so some generic phrasing
      k_log_event("Failed to load STARTUP_PROG \"%s\"",
        startup_prog);
      k_exit(1);
    }
  } else {
#else
  {
#endif
    k_add_named_thread_with_args(setup, NULL,
      &empty_args, 0);
  }

  load_first_thread();

  __builtin_unreachable();
}

static void inc_msg_pointer(Thread* thr, Message** ptr) {
  ++(*ptr);
  // Wrap around from the end to the start
  if (*ptr == &(thr->messages[THREAD_MSG_QUEUE_SIZE])) {
    *ptr = &(thr->messages[0]);
  }
}

bool k_get_msg(int* sender, int* message) {
  // If message box is not empty, or it is full
  if (_current_thread->next_msg != _current_thread->end_msgs ||
      _current_thread->msgs_full) {
    *sender = _current_thread->next_msg->src;
    *message = _current_thread->next_msg->content;

    inc_msg_pointer(_current_thread, &_current_thread->next_msg);
    _current_thread->msgs_full = false;

    return true;
  }

  return false;
}

bool k_send_msg(int destination, int message) {
  if (
      // Invalid destination
      destination >= MAX_THREADS || destination < 0 ||
      all_threads[destination].id == INVALID_THREAD ||
      // Buffer is full
      all_threads[destination].msgs_full) {
    return false;
  }

  Thread* dest = &all_threads[destination];
  Message* our_msg = dest->end_msgs;
  our_msg->src = k_get_thread_id();
  our_msg->content = message;
  inc_msg_pointer(dest, &(dest->end_msgs));
  dest->msgs_full = dest->next_msg == dest->end_msgs;

  return true;
}

void k_log_event(const char* event, ...) {
  if (!(kernel_config & KCFG_LOG_THREADS)) {
    return;
  }

  char thread_name[THREAD_NAME_SIZE + 1];
  const char* name = NULL;
  if (_current_thread) {
    name = _current_thread->name;
  }
  format_thread_name(thread_name, k_get_thread_id(), name);
  printf("Thread %s: ", thread_name);

  va_list args;
  va_start(args, event);
  vprintf(event, args);
  va_end(args);

  printf("\n");
}

void log_scheduler_event(const char* event) {
  if (kernel_config & KCFG_LOG_SCHEDULER) {
    printf("Thread  <scheduler>: %s\n", event);
  }
}

#if CODE_BACKING_PAGES
static void swap_paged_threads(const Thread* current, const Thread* next) {
    // See if we need to swap out current thread
    if (current && (current->code_backing_page != INVALID_PAGE)) {
      memcpy(code_page_backing[current->code_backing_page],
             code_page, CODE_PAGE_SIZE);
    }
    // If the next thread's code is in a backing page
    if (next->code_backing_page != INVALID_PAGE) {
      memcpy(code_page,
             code_page_backing[next->code_backing_page],
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

void do_scheduler(void) {
  // NULL next_thread means choose one for us
  // otherwise just do required housekeeping to switch
  if (next_thread != NULL) {
#if CODE_BACKING_PAGES
    swap_paged_threads(_current_thread, next_thread);
#endif
    return;
  }

  size_t start_thread_idx = next_possible_thread_idx(_current_thread);

  size_t max_thread_idx = start_thread_idx + MAX_THREADS;
  for (size_t idx = start_thread_idx; idx < max_thread_idx; ++idx) {
    // Wrap into range of array
    size_t _idx = idx % MAX_THREADS;

    if (!can_schedule_thread(_idx)) {
      continue;
    }

    if (all_threads[_idx].id != _idx) {
      printf("thread ID %u and position %u inconsistent!\n", (unsigned)_idx, all_threads[_idx].id);
      k_exit(1);
    }

    log_scheduler_event("scheduling new thread");

    log_scheduler_event("next thread chosen");
    next_thread = &all_threads[_idx];

#if CODE_BACKING_PAGES
    swap_paged_threads(_current_thread, next_thread);
#endif

    check_signals(next_thread);

    return;
  }

  // If the current thread is the last one, just return to it
  if (can_schedule_thread(k_get_thread_id())) {
    next_thread = _current_thread;
  } else {
    log_scheduler_event("all threads finished");
    k_exit(0);
  }
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
  bool set = set_thread_state(tid, cancelled);
  if (set) {
    cleanup_thread(&all_threads[tid]);
  }
  return set;
}

static void thread_switch(void) {
  asm volatile("svc %0" : : "i"(svc_thread_switch));
}

void check_stack(void);
static bool k_do_yield(Thread* to) {
  check_stack();

  next_thread = to;
  do_scheduler();
  // Assembly handler will see next_thread set and do the switch
  return true;
}

static bool k_yield_next(void) {
  // Yield to next valid thread, wrapping around the list
  // Pretty much what the scheduler does, but you will return
  // to current thread if there isn't another one to go to
  int id = k_get_thread_id();
  bool found = false;

  // Check every other thread than this one
  size_t limit = id + MAX_THREADS;
  for (size_t idx = id + 1; idx < limit; ++idx) {
    size_t idx_in_range = idx % MAX_THREADS;
    if (can_schedule_thread(idx_in_range)) {
      k_do_yield(&all_threads[idx_in_range]);
      found = true;
      break;
    }
  }

  // If we set next_thread then we'll switch after this return
  // If not, back to the same thread
  return found;
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
    case YIELD_NEXT:
      assert(tid == INVALID_THREAD);
      return k_yield_next();
    default:
      assert(0);
      break;
  }

  __builtin_unreachable();
}

#if CODE_PAGE_SIZE && ! CODE_BACKING_PAGES
static int code_page_in_use_by() {
  for (size_t idx=0; idx<MAX_THREADS; ++idx) {
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
  for (size_t i=0; i<CODE_BACKING_PAGES; ++i) {
    possible[i] = true;
  }
  for (size_t i=0; i<MAX_THREADS; ++i) {
    size_t page = all_threads[i].code_backing_page;
    if (page != INVALID_PAGE) {
      possible[page] = false;
    }
  }
  for (size_t i=0; i<CODE_BACKING_PAGES; ++i) {
    if (possible[i]) {
      return i;
    }
  }
  return INVALID_PAGE;
}
#endif

#if CODE_PAGE_SIZE
static void setup_code_page(size_t idx) {
  Thread* curr = _current_thread;
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
                               const ThreadArgs* args, uint16_t remove_permissions) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    if (all_threads[idx].id == INVALID_THREAD ||
        all_threads[idx].state == cancelled ||
        all_threads[idx].state == finished) {

      // New thread's permissions is the current thread's
      // minus any it wants to remove
      uint16_t permissions = TPERM_ALL;
      if (_current_thread) {
        permissions = _current_thread->permissions;
      }
      permissions &= ~remove_permissions;

      init_thread(&all_threads[idx], idx, name,
                  worker, args, permissions);
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
    if (_current_thread) {
      _current_thread->err_no = E_NO_PAGE;
    }
    return INVALID_THREAD;
  }

  uint8_t* dest = code_page;
#if CODE_BACKING_PAGES
  dest = &code_page_backing[free_page][0];
#endif

  void (*entry)() = load_elf(filename, dest);
  if (!entry) {
    // Startup may call this to load STARTUP_PROG
    if (_current_thread) {
      _current_thread->err_no = E_NOT_FOUND;
    }
    return INVALID_THREAD;
  }
  int tid = k_add_named_thread_with_args(
    entry, filename, args, remove_permissions);

#if CODE_BACKING_PAGES
  all_threads[tid].code_backing_page = free_page;
#else
  all_threads[tid].in_code_page = true;
#endif
  return tid;
}
#endif

int k_add_thread(const char* name,
                 const ThreadArgs* args,
                 void* worker,
                 uint32_t flags) {
  if (k_has_no_permission(TPERM_CREATE)) {
    return -1;
  }

  ThreadArgs dummy_args = {0,0,0,0};
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
      return k_add_thread_from_file_with_args(
              (const char*) worker, args, remove_permissions);
#endif
    case THREAD_FUNC:
      return k_add_named_thread_with_args(worker, name,
        args, remove_permissions);
    default:
      assert(0 && "invalid flags!");
      break;
  }

  __builtin_unreachable();
}

void k_thread_wait(void) {
  _current_thread->state = waiting;
  // Manually move to next thread
  do_scheduler();
}

// Use these struct names to ensure that these are
// placed *after* the thread structs to prevent
// stack overflow corrupting them.
__attribute__((section(".thread_vars"))) size_t thread_stack_offset =
    offsetof(Thread, stack);

#ifdef __thumb__
extern void thread_switch_from_kernel_mode(void);
#endif
void check_stack(void) {
  bool underflow = _current_thread->bottom_canary != STACK_CANARY;
  bool overflow = _current_thread->top_canary != STACK_CANARY;

  if (underflow || overflow) {
    // Don't schedule this again, or rely on its ID
    _current_thread->id = INVALID_THREAD;
    _current_thread->name = NULL;

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
      _current_thread->id = INVALID_THREAD;

      // Would clear heap allocs here but we can't trust the thread ID

      // Aka don't save any state, just load the scheduler
      // TODO: this isn't actually checked by the assembly
      _current_thread->state = init;
#ifdef __thumb__
      // thumb doesn't like SVC in kernel mode
      thread_switch_from_kernel_mode();
#else
      thread_switch();
#endif
    } else {
      k_exit(1);
    }
  }
}

__attribute__((noreturn)) void thread_start(void) {
  // Every thread starts by entering this function

  // Call thread's actual function
  _current_thread->work(_current_thread->args.a1, _current_thread->args.a2,
                         _current_thread->args.a3, _current_thread->args.a4);

  // Yield back to the scheduler
  k_log_event("exiting");

  // Make sure we're not scheduled again
  _current_thread->state = finished;

  /* You might think this is a timing issue.
     What if we're interrupted here?

     Well, we'd go to thread_switch, next_thread
     is set to the scheduler automatically.
     Since our state is finished, it won't be updated
     to suspended. Meaning, we'll never come back here.

     Which is just fine, since we were going to switch
     away anyway.
  */

  cleanup_thread(_current_thread);

  // Calling thread_switch directly so we don't print 'yielding'
  thread_switch();

  __builtin_unreachable();
}

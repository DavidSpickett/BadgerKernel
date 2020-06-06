#ifdef linux
#define _GNU_SOURCE
#include <pthread.h>
#endif
#include "print.h"
#include "thread.h"
#include "util.h"
#if CODE_PAGE_SIZE
#include "elf.h"
#endif
#include <string.h>
#include <stdarg.h>

#ifndef linux
#include "alloc.h"
#endif

__attribute__((section(".thread_vars"))) Thread* _current_thread;

__attribute__((section(".thread_structs"))) Thread all_threads[MAX_THREADS];
// Volatile is here for the pthread implementation
__attribute__((section(".thread_vars"))) Thread* volatile next_thread;

__attribute__((section(".thread_vars")))
KernelConfig kernel_config = {.destroy_on_stack_err = false,
                        .log_scheduler = true};

#if CODE_PAGE_SIZE
__attribute__((section(".code_page"))) uint8_t code_page[CODE_PAGE_SIZE];
#if CODE_BACKING_PAGES
#define INVALID_PAGE 123456
__attribute__((section(".code_page_backing")))
uint8_t code_page_backing[CODE_BACKING_PAGES][CODE_PAGE_SIZE];
#endif
#endif /* CODE_PAGE_SIZE */

void k_set_kernel_config(const KernelConfig* config) {
  kernel_config = *config;
}

bool is_valid_thread(int tid) {
  return (tid >= 0) && (tid < MAX_THREADS) && all_threads[tid].id != -1;
}

static bool can_schedule_thread(int tid) {
  return is_valid_thread(tid) && \
    (all_threads[tid].state == suspended ||
     all_threads[tid].state == init ||
     all_threads[tid].state == running);
}

bool k_get_thread_state(int tid, ThreadState* state) {
  if (is_valid_thread(tid)) {
    *state = all_threads[tid].state;
    return true;
  }
  return false;
}

Thread* current_thread(void);

void k_invalid_syscall(void) {
  printf("Unknown syscall invoked!\n");
  exit(1);
}

const char* k_get_thread_name(void) {
  return current_thread()->name;
}

int k_get_thread_id(void) {
  Thread* curr = current_thread();
  return curr ? curr->id : -1;
}

void init_thread(Thread* thread, int tid, const char* name,
                 void (*do_work)(void), const ThreadArgs* args) {
  // thread_start will jump to this
  thread->work = do_work;
  thread->state = init;

  thread->id = tid;
  thread->name = name;
  thread->args = *args;

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

#ifndef linux
  thread->bottom_canary = STACK_CANARY;
  thread->top_canary = STACK_CANARY;
  // Top of stack
  size_t stack_ptr = (size_t)(&(thread->stack[THREAD_STACK_SIZE]));
  // Mask to align to 16 bytes for AArch64
  thread->stack_ptr = (uint8_t*)(stack_ptr & ~0xF);
#endif
}

#ifdef linux
pthread_mutex_t first_schedule_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scheduler_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
extern void setup(void);
void do_scheduler(void);
extern void start_thread_switch(void);
__attribute__((noreturn)) void entry(void) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    ThreadArgs noargs = {0, 0, 0, 0};
    init_thread(&all_threads[idx], -1, NULL, NULL, &noargs);
  }

#ifdef linux
  // Stop initial threads running yet
  pthread_mutex_lock(&first_schedule_mutex);
#endif

  setup(); // Adds initial threads

#ifdef linux
  do_scheduler();
  pthread_mutex_unlock(&first_schedule_mutex);
  while (1) { //!OCLINT
  }
#else
  start_thread_switch(); // Not thread_switch as we're in kernel mode
#endif
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
  if (current_thread()->next_msg != current_thread()->end_msgs ||
      current_thread()->msgs_full) {
    *sender = current_thread()->next_msg->src;
    *message = current_thread()->next_msg->content;

    inc_msg_pointer(current_thread(), &current_thread()->next_msg);
    current_thread()->msgs_full = false;

    return true;
  }

  return false;
}

bool k_send_msg(int destination, int message) {
  if (
      // Invalid destination
      destination >= MAX_THREADS || destination < 0 ||
      all_threads[destination].id == -1 ||
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

#ifdef linux
void thread_switch(void);
#else
extern void thread_switch(void);
void check_stack(void);
#endif

void k_format_thread_name(char* out) {
  // fill with spaces (no +1 as we'll terminate it later)
  for (size_t idx = 0; idx < THREAD_NAME_SIZE; ++idx) {
    out[idx] = ' ';
  }

  const char* name = current_thread()->name;

  if (name == NULL) {
    int tid = k_get_thread_id();

    // If the thread had a stack issue
    if (tid == -1) {
      const char* hidden = "<HIDDEN>";
      size_t h_len = strlen(hidden);
      size_t padding = THREAD_NAME_SIZE - h_len;
      strncpy(&out[padding], hidden, h_len);
    } else {
      // Just show the ID number (assume max 999 threads)
      char idstr[4];
      int len = sprintf(idstr, "%u", tid);
      strcpy(&out[THREAD_NAME_SIZE - len], idstr);
    }
  } else {
    size_t name_len = strlen(name);

    // cut off long names
    if (name_len > THREAD_NAME_SIZE) {
      name_len = THREAD_NAME_SIZE;
    }

    size_t padding = THREAD_NAME_SIZE - name_len;
    strncpy(&out[padding], name, name_len);
  }

  out[THREAD_NAME_SIZE] = '\0';
}

void k_log_event(const char* event, ...) {
  char thread_name[THREAD_NAME_SIZE + 1];
  k_format_thread_name(thread_name);
  printf("Thread %s: ", thread_name);

  va_list args;
  va_start(args, event);
  vprintf(event, args);
  va_end(args);

  printf("\n");
}

void log_scheduler_event(const char* event) {
  if (kernel_config.log_scheduler) {
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
    // +1 to skip the current thread
    return curr - &all_threads[0] + 1;
  }

  // On startup current_thread will be NULL
  return 0;
}

void do_scheduler(void) {
#ifdef linux
  pthread_mutex_lock(&scheduler_mutex);
#endif

  // NULL next_thread means choose one for us
  // otherwise just do required housekeeping to switch
  if (next_thread != NULL) {
#if CODE_BACKING_PAGES
    swap_paged_threads(current_thread(), next_thread);
#endif
    return;
  }

  Thread* curr = current_thread();
  size_t start_thread_idx = next_possible_thread_idx(curr);

  size_t max_thread_idx = start_thread_idx + MAX_THREADS;
  for (size_t idx = start_thread_idx; idx < max_thread_idx; ++idx) {
    // Wrap into range of array
    size_t _idx = idx % MAX_THREADS;

    if (!can_schedule_thread(_idx)) {
      continue;
    }

    if (all_threads[_idx].id != _idx) {
      printf("thread ID %u and position %u inconsistent!\n", (unsigned)_idx, all_threads[_idx].id);
      exit(1);
    }

    log_scheduler_event("scheduling new thread");

    log_scheduler_event("next thread chosen");
    next_thread = &all_threads[_idx];

#if CODE_BACKING_PAGES
    swap_paged_threads(curr, next_thread);
#endif

#ifdef linux
    pthread_mutex_unlock(&scheduler_mutex);
#endif

    return; //!OCLINT
  }

  // If the current thread is the last one, just return to it
  if (can_schedule_thread(k_get_thread_id())) {
    next_thread = _current_thread;
  } else {
    log_scheduler_event("all threads finished");
    exit(0);
  }
}

static bool set_thread_state(int tid, ThreadState state) {
  if (is_valid_thread(tid)) {
    all_threads[tid].state = state;
    return true;
  }
  return false;
}

bool thread_wake(int tid) {
  return set_thread_state(tid, suspended);
}

static void cleanup_thread(Thread* thread) {
#ifdef linux
  // If this is called when a thread exits, we don't
  // want to cancel ourselves, just exit normally.
  if (thread != current_thread()) {
    pthread_cancel(thread->self);
  }
#else
  // Free any lingering heap allocations
  free_all(thread->id);

#if CODE_PAGE_SIZE
  // Free the code page. If there are other threads
  // running from it they will still have true to keep it alive.
  thread->in_code_page = false;
#if CODE_BACKING_PAGES
  thread->code_backing_page = INVALID_PAGE;
#endif
#endif
#endif /* ifdef linux */
}

bool thread_cancel(int tid) {
  bool set = set_thread_state(tid, cancelled);
  if (set) {
    cleanup_thread(&all_threads[tid]);
  }
  return set;
}

void k_thread_yield(Thread* next) {
  // TODO: fix linux port, ergh
#ifndef linux
  check_stack();
#endif

  next_thread = next;
  do_scheduler(); // TODO: ensure scheduler is called whenever next is set
  // Assembly handler will see next set and
  // do a switch
}

bool k_yield_to(int tid) {
  if (!can_schedule_thread(tid)) {
    return false;
  }

  k_thread_yield(&all_threads[tid]);
  return true;
}

bool k_yield_next(void) {
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
      k_thread_yield(&all_threads[idx_in_range]);
      found = true;
      break;
    }
  }

  // If we set next_thread then we'll switch after this return
  // If not, back to the same thread
  return found;
}

// TODO: these ifdefs are a bit much
#if CODE_PAGE_SIZE && ! CODE_BACKING_PAGES
static int code_page_in_use_by() {
  for (size_t idx=0; idx<MAX_THREADS; ++idx) {
    if (all_threads[idx].in_code_page) {
      return all_threads[idx].id;
    }
  }
  return -1;
}
#endif

#if CODE_BACKING_PAGES
static size_t find_free_backing_page() {
  //TODO: not very efficient
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
int k_add_thread_from_file(const char* filename) {
#if CODE_BACKING_PAGES
  size_t free_page = find_free_backing_page();
  // If we have backing, don't count the active
  // page as useable for loading.
  if (free_page == INVALID_PAGE) {
#else
  if (code_page_in_use_by() != -1) {
#endif
    return -1;
  }

  int file = open(filename, O_RDONLY);
  if (file < 0) {
    printf("Couldn't load %s\n", filename);
    exit(1);
  }

  uint8_t* dest = code_page;
#if CODE_BACKING_PAGES
  dest = &code_page_backing[free_page][0];
#endif

  int tid = k_add_named_thread(load_elf(filename, dest), filename);

#if CODE_BACKING_PAGES
  all_threads[tid].code_backing_page = free_page;
#else
  all_threads[tid].in_code_page = true;
#endif
  return tid;
}
#endif

int k_add_thread(void (*worker)(void)) {
  return k_add_named_thread(worker, NULL);
}

int k_add_named_thread(void (*worker)(), const char* name) {
  ThreadArgs args = {0, 0, 0, 0};
  return k_add_named_thread_with_args(worker, name, &args);
}

#if CODE_PAGE_SIZE
static void setup_code_page(size_t idx) {
  Thread* curr = current_thread();
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

#ifdef linux
void* thread_entry();
#endif
int k_add_named_thread_with_args(void (*worker)(), const char* name,
                               const ThreadArgs* args) {
  for (size_t idx = 0; idx < MAX_THREADS; ++idx) {
    if (all_threads[idx].id == -1 ||
        all_threads[idx].state == cancelled ||
        all_threads[idx].state == finished) {
      init_thread(&all_threads[idx], idx, name, worker, args);
#ifdef linux
      pthread_create(&all_threads[idx].self, NULL, thread_entry, NULL);
#endif
#if CODE_PAGE_SIZE
      setup_code_page(idx);
#endif
      return idx;
    }
  }
  return -1;
}

void thread_wait(void) {
  current_thread()->state = waiting;
  // Skip the yielding print
  next_thread = NULL;
  thread_switch();
}

#ifdef linux

Thread* current_thread(void) {
  pthread_t self = pthread_self();
  for (int i = 0; i < MAX_THREADS; ++i) {
    if (all_threads[i].self == self) {
      return &all_threads[i];
    }
  }
  return NULL;
}

void* thread_entry() {
  // Hold back threads until first run of scheduler is done
  pthread_mutex_lock(&first_schedule_mutex);
  pthread_mutex_unlock(&first_schedule_mutex);

  // Then wait until we're chosen by scheduler to run
  while (next_thread != current_thread()) {
    pthread_yield();
  }

  // This is here to force a check for pending cancels
  // (best I could come up with)
  usleep(1);

  current_thread()->work(current_thread()->args.a1, current_thread()->args.a2,
                         current_thread()->args.a3, current_thread()->args.a4);

  // Yield back to the scheduler
  k_log_event("exiting");

  // Make sure we're not scheduled again
  current_thread()->state = finished;

  cleanup_thread(current_thread());

  // Must call this so we check if there are threads left
  next_thread = NULL;
  do_scheduler();

  return NULL; //!OCLINT
}

void thread_switch(void) {
  if (!next_thread) {
    do_scheduler();
  }
  while (next_thread != current_thread()) {
    pthread_yield();
  }
}

#else

// Use these struct names to ensure that these are
// placed *after* the thread structs to prevent
// stack overflow corrupting them.
__attribute__((section(".thread_vars"))) size_t thread_stack_offset =
    offsetof(Thread, stack);

Thread* current_thread(void) {
  return _current_thread;
}

void stack_extent_failed(void) {
  // current_thread is likely still valid here
  k_log_event("Not enough stack to save context!");
  exit(1);
}

void check_stack(void) {
  bool underflow = current_thread()->bottom_canary != STACK_CANARY;
  bool overflow = current_thread()->top_canary != STACK_CANARY;

  if (underflow || overflow) {
    // Don't schedule this again, or rely on its ID
    current_thread()->id = -1;
    current_thread()->name = NULL;

    if (underflow) {
      k_log_event("Stack underflow!");
    }
    if (overflow) {
      k_log_event("Stack overflow!");
    }

    if (kernel_config.destroy_on_stack_err) {
     /* Setting -1 here, instead of state=finished is fine,
         because A: the thread didn't actually finish
                 B: the thread struct is actually invalid */
      current_thread()->id = -1;

      // Would clear heap allocs here but we can't trust the thread ID

      // Aka don't save any state, just load the scheduler
      current_thread()->state = init;
      thread_switch();
    } else {
      exit(1);
    }
  }
}

// TODO: it's kinda weird that this is in kernel
// Maybe threads should just enter directly to their
// worker?
// Mind you, only the kernel *references* this
// so that mitigates it some
__attribute__((noreturn)) void thread_start(void) {
  // Every thread starts by entering this function

  // Call thread's actual function
  current_thread()->work(current_thread()->args.a1, current_thread()->args.a2,
                         current_thread()->args.a3, current_thread()->args.a4);

  // Yield back to the scheduler
  k_log_event("exiting");

  // Make sure we're not scheduled again
  current_thread()->state = finished;

  /* You might think this is a timing issue.
     What if we're interrupted here?

     Well, we'd go to thread_switch, next_thread
     is set to the scheduler automatically.
     Since our state is finished, it won't be updated
     to suspended. Meaning, we'll never come back here.

     Which is just fine, since we were going to switch
     away anyway.
  */

  cleanup_thread(current_thread());

  // Calling thread_switch directly so we don't print 'yielding'
  // TODO: we save state here that we don't need to
  thread_switch();

  __builtin_unreachable();
}

#endif // ifndef linux

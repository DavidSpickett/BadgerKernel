# Badger Kernel Kernel design notes

This document describes how key parts of the kernel work and why.

# Goals
* Learn about kernel and OS concepts
* Make a "real" but compact project to experiment with toolchains and testing utilities.
* Write some C. Why C? I like it, mostly, and when I don't it gives me things to complain about, which I do like.
* Pass all tests with as wide a set of compiler checks as possible
* Adding a new platform should be easy and rebasing that port should be conflict free as much as possible.
* Implementing "libc" user space features sometimes

# Non Goals

(except where it's fun to do so)

* Innovating new kernel concepts
* Having a "real world" use case (or indeed users)
* Running on real hardware
* Being secure
* Being fast
* Porting existing software to Badger Kernel
* Providing a stable system ABI
* Being any particular kind of kernel
* Having an exhaustive test suite
* Keeping this document up to date :)

# Origins

Badger Kernel is the product of boredom and curiosity. Most of the initial structure is influenced by my experience doing debug support for RTOS such as Zephyr and FreeRTOS.

The cycle always goes like this:
* Read a high level overview of a thing
* Decide how you *think* it works
* Implement that
* Have reality tell you that you're wrong
* Make adjustments

Somewhere along the line you might even read the implementation from another system. I find it fun to make some mistakes first though.

# Project Layout

This describes the source level layout of Badger Kernel.

* .github/workflows - Config for Github Actions CI
* cmake - CMake functions for adding demos and loadable files
* demos - Test applications
* include
    - common (used in kernel and user code)
    - kernel (only used in kernel code)
    - port   (per platform functions)
    - user   (only included in user code)
* linker - linker scripts for the kernel and loadable programs
* scripts - Random non code stuff like Python cog functions
* src
    - common (used in kernel and user)
    - hw (per platform code)
    - kernel (the kernel itself)
    - user (user space code, the "libc" parts if you will)

# Tools

These tools are used for the development and testing:
* cmake
* make
* gcc
* lit (llvm's test runner)
* expect
* Python cog module (for code generation)
* clang-format
* QEMU

# Demos

Demos are the tests for Badger Kernel. They aren't meant to be exhaustive but I try to cover the obvious corner cases. Hence "demos" not "tests", not that it matters much.

The general idea was to make the early demos use as few features as possible so that you could start there when porting a new platform.

Over time that has changed, since even the yielding demo (the first one written) requires syscalls for instance.

Demos mostly use a golden reference log file. That works like this:
* Run the demo in QEMU with serial into a file
* Compare that file with demos/\<demo\>/expected.log
* Any difference is a test failure

The exception (for now) is the backtrace demo. Functions addresses change with compiler settings so we can't use a static logfile.

Instead we use expect's regex abilities.
* Run the demo in QEMU with serial into a file
* Run demos/\<demo\>/expected.exp which cats the previous file and expects some lines

## Testing the Demos

There are three ways to run the demos.

1. `make test_\<demo\>` to check a single demo
2. `make` will build and test all demos
3. `lit demos/` to run the tests with lit (this is what CI does)

Running with lit provides better feedback in general but make is more convenient or development.

# Kernel Structure

## Key Variables

There are 3 key objects for the kernel:
* `all_threads` - an array of thread structures
* `current_thread` - pointer to the active thread
* `next_thread` - a pointer to the next thread to be ran

`all_threads` is an array of `Thread` structures that is statically allocated with size `MAX_THREADS`. So at any one time some of them may not be valid threads.

The "scheduler" is very simple it just finds the next thread that is available to run and sets next_thread to that. (plus some housekeeping)

The most important parts of the Thread structure are:
* `stack_ptr` - It's last sp value. From the kernel's point of view this always points to the start of the thread's saved context
* `state` - The state of the thread. Most of the time it will jump between suspended and running, the full list is:
    - `init` (not run at all yet)
    - `running` (is the active thread)
    - `suspended` (not running, but can be chosen to run next)
    - `waiting` (cannot run until some condition is met)
    - `finished`
    - `cancelled`
* `id` - Used to identify the thread in log messages and find its address in all_threads. For valid threads it's index will always be the same as its ID.
* `stack` - The stack memory for the thread. stack_ptr will point to somewhere in here.

## Kernel Startup

(varies per platform but you get the idea)

1. Setup interrupt routing
2. Init kernel mode stack pointer
3. Jump into kernel C code
4. Initialise all_threads, making them all invalid
5. Add a setup thread as thread 0
6. Run the scheduler, which will pick the setup thread
7. Load the setup thread (which puts us in user mode)
8. Run user setup code

The reason for adding the setup thread is akin to running "init" in other systems. I used to have the initial user threads added in a kernel mode function but didn't like having kernel functions in user demos.

Hence setup. The user defines `setup()` and the setup thread runs that. You can do anything you want in that function, including real code and yielding. Without any special case code in the kernel.

## Saving/Restoring Threads

Note: I use the term "thread" because Badger Kernel doesn't have a concept of processes.

This isn't any kind of grand statement, it's just because the RTOS I learned from also don't have processes. You could say a "thread" for Badger Kernel is as heavyweight as a process would be elsewhere.

### Background

Badger Kernel's switching is pretty much all cooperative. So we know when a thread is going to yield. However this mostly me being too lazy to handle interrupts properly.

With that in mind thread switching saves everything, as if it were pre-emptive. (see the `selfyield` demo for a test of this)

The reason to mention this is that if you know your switches only come from function calls, you can simply save the callee saved registers from your ABI.
```
<user code>
yield();
<more user code>
```
In the code above saving callee only would be fine since the compiler sees yield as a function call.
```
<user code>
asm volatile("svc 123");
<more user code>
```
For the code above you need to save *everything* because the compiler isn't going to know that that asm is basically a function call. (a syscall if you will)

Anyway, the upshot is that Badger Kernel saves all user registers on kernel entry. Which is not totally needed and is slower but:
1. It gives us some cool features like modifying thread registers, without some 2 stage saving process.
2. Calling kernel C functions is as easy as...well, calling them.
3. We can sort of support timer interrupts
4. Speed is not a goal

### Thread Context Content

The following are saved onto the thread's stack:
* All general purpose registers
* Special flag registers like Arm's PSR
* The program counter (read from some exception handling register)

The context *does not* include the stack pointer. That is stored elsewhere in the thread structure.

You can think of this context as an extra stack frame. For instance:
```
void foo(void) { yield();}
void bar(void) { foo(); }
void my_thread(void) {
    bar();
}
```
Would produce a stack like:
```
stack_top (high address)
my_thread
bar
foo
yield
saved context
<<< thread->stack_ptr
```
Where the thread's `stack_ptr` points to the start of the context. (you can see these contexts in `include/port/<platform>.h`)

### The Initial Thread Context

To remove special cases, even threads that haven't run yet have a context on their stack. This context is zeroed apart from:
* The address of the `thread_start` function
* Any special configuration register values the platform needs (e.g. Thumb mode)

### Thread Lifecycle Example

This walks you through the following:
* Starting the system
* Running the setup thread
* yielding back to the kernel
* Finishing the thread
* Exiting the system

First the kernel adds the setup thread. This means setting its worker function to `setup()` as defined by the user.

Note that all threads start in `thread_start` which *then* calls the worker function. This allows us to catch finished threads.

Then the kernel jumps into the middle of the thread switching code (see `yield.S`), skipping the save thread portion to get to thread loading.

The setup thread's stack looks like this:
```
stack top (high address)
initial register context
<<< stack_ptr (low address)
```
The loading code gets the thread's stack pointer from the thread structure and uses it to restore the context into the real registers.

Then we exception return, which starts the user thread at the desired pc. (remember that reset is usually an exception too so we can still "return from" it)

Now `setup()` executes. It's like any other thread so any user code can be run but for now assume we just yield.
```
void setup(void) {
    yield();
}
```
`yield()` will end up causing an exception that takes us into the exception handler.

The first thing this does is save the user thread's registers. So it's stack now looks like:
```
stack top (high address)
thread_start()
setup()
yield()
<misc user code wrappers>
saved context
<<< stack_ptr (low address)
```
Then the kernel checks why we got here. It's either:
* a timer interrupt
* plain supervisor call
* a syscall supervisor call

Here it's the third one. So we jump into the kernel's syscall handler which finds that it's `yield`.

Yield causes us to run the scheduler to pick the next thread. Thread 0 is our only valid thread so we keep walking `all_threads` until we hit the end, wrap around and get back to 0.

So `next_thread` is set to thread 0 and we return from the syscall.

Side note here about syscalls is that whenever we return from the syscall handler with a `next_thread` that is not NULL, we use that value.

If it were NULL then we would simply return to the same thread. Which in this example is the same result.

Now we have chosen a thread so we restore it by loading the real registers with the values in its stored context. Return from exception and now we're:
```
void setup(void) {
    yield();
    <<< here >>>
}
```
`setup()` continues to run and returns to `thread_start`. Now we know the thread has ended so we set our state to finished and yield again. We will not be rescheduled due to our state.

Back in the kernel we save the thread context (not required but again, speed not a goal) then run the scheduler.

The scheduler walks the whole of `all_threads` and doesn't find any valid threads so the system exits.

# Syscalls

Badger Kernel's syscalls are generated from a file `scripts/syscalls.py` into a header `include/common/syscall.h`. (amongst other places)

The general idea is to keep the list minimal so some like `mutex` are a single call with a flag to define what it does.
(yes mutexes can be done in user space but I'm lazy and making it a syscall is an easy way to get atomicity)

The other example of this is `get_thread_property` and `set_thread_property`. These are used to implement a bunch of calls that are all reading/writing bits of the thread structure without doing anything else.
(e.g sending a signal uses `set_thread_property`)

## User Syscall Usage

User threads invoke syscalls with their platform's supervisor call instruction. There is a set supervisor call number that means syscalls. (some will have to put this in a register if their instruction doesn't have a code field)

Arguments are loaded into the first 4 argument registers, and the syscall number into some other register. `src/user` provides wrappers for each syscall that in turn call `generic_syscall`. This interface is implemented by each platform.

For example mutex is a single call as mentioned but has multiple actions. So there is a function for each operation that calls the generic invoker with the right arguments.

When a user thread returns from a syscall the result is in the first argument register.

## Kernel Syscall Handling

The exception handler will, in some platform specific way, call `k_handle_syscall`.

This uses the thread's saved context to load the 4 possible arguments and the syscall number. Then switches on the number and calls the kernel function for that syscall.

We save the result and then store that in the thread's saved register context. So as far as it's concerned, only the first argument register has changed.

As a security (the fun kind of security) measure we zero out the result if the syscall does not have a return value. Imagine:
```
int i = user_syscall_foo();
<...into the kernel...>
void k_foo() {
    <do some stuff with kernel pointers>
}
<oops, r0 has some internal pointer in it>
<return to user>
<now the user has that pointer>
```

# Signal Handling

A signal is just a number that is stored as a bitmask on the thread. So you can only have one pending signal per number, anything else is ignored.

The scheduler will check the next thread for pending signals. Lower signal numbers (lower bits) are handled first.

Where handling means the next time the thread would run it runs its handler function (if it has one) then returns to the kernel. Meaning that a signal makes a thread skip its turn for normal execution.

There is only one user signal handler and it gets the signal number as its first argument. If you don't have a handler registered, all signals are ignored.

Getting the thread to execute the handler function is done by:
* Initialising another register context on its stack
```
stack top (high address)
thread_start();
thread_function();
<whatever it was doing>
yield();
<saved register context>
<initialised register context>
```
* Setting the pc of that new context to a special wrapper function `signal_handler_wrapper`.
* Resuming the thread
* Which runs `signal_handler_wrapper`
* Which calls the user's signal handler function
* The handler returns to `signal_handler_wrapper`
* Which does a yield supervisor call (instead of the syscall version)

Now that we've handled that signal we can remove the saved context for that, bump the stack pointer up back to the thread's first saved context and resume from there.
```
stack top (high address)
thread_start();
thread_function();
<whatever it was doing>
yield();
<saved register context>
<<< new stack_ptr
<signal handling register context>
<<< old_stack_ptr
```

Ok, but why do we need this wrapper to call the handler? Why not just jump right into the handler?

We could just assume that when the handler returns, handling is finished. In fact, we do, but how do we know when it's returned?

This handler could be anywhere in memory, and I don't
want to complicate user code with some macro or required yield calls.

Instead we arrange for the handler to return to a function we control the location of. More importantly
we control the location of the yield call in that function. So the callstack looks like:
```
signal_handler_wrapper
user_signal_handler
```

We define a special symbol in `signal_handler_wrapper`
that marks the location of the yield syscall. The user handler returns, we do the syscall and the kernel sees this magic address.

That's how we know handling has finished and we can remove the context.

## Handling multiple signals

What would happen if you were handling a signal, you yielded from the user handler and we found another one waiting?

Well, you would start handling that one instead. Each time we see you yield from the magic handling ended address we remove a signal handling context. For example:
```
stack top (high address)
<user functions>
yield()
<user context>
<signal 0 handler>
yield()
<signal 0 context>
<signal 1 handler>
```

Above is the stack during one of these situations. (note that `signal_handler_wrapper` deliberately doesn't use any stack space)

Assume handling of signal 1 finishes. The kernel sees this:
```
stack top (high address)
<user functions>
yield()
<user context>
<signal 0 handler>
yield()
<signal 0 context>
<signal 1 context>
```

Where the saved PC is the magic end address. So we bump the stack pointer up, back to signal 0's saved context.

```
stack top (high address)
<user functions>
yield()
<user context>
<signal 0 handler>
yield()
<signal 0 context>
<<< new stack_ptr
<signal 1 context>
<<< old stack_ptr
```

Then we resume the thread, which restores signal 0's context. Then signal 0 handling finishes.

```
stack top (high address)
<user functions>
yield()
<user context>
<signal 0 context>
```

Once again we have the magic address so we remove a context from the stack.

```
stack top (high address)
<user functions>
yield()
<user context>
<<< new stack_ptr
<signal 0 context>
<<< old stack_ptr
```

Then we resume the user thread and it continues as normal.

They key point is that we don't need to know which signal each context refers to. We simply remove a saved context each time we see the magic end address.

# TODO: user threads

# TODO: CI setup

# TODO: binary loading

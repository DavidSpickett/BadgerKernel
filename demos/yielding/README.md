# Yielding

In this demo we show the usage of yielding, which occurs in a computer program during multithreading.

By calling syscall `yield`, the thread gives away the utilization of the CPU to another thread, or a specific thread (discussed in another demo).

To describe thread yielding, we need to introduce:

* `add_thread_from_worker` (in `src/user/thread.c`)
* `do_scheduler` (in `src/kernel/thread.c`)
* `yield` (in `src/user/thread.c`)

## `add_thread_from_worker`

When a thread calls `add_thread_from_worker`, the kernel will create a new thread with the given arguments, initialize the permissions, and set its starting point to be the given function (which is `thread_worker()` in this demo).

After the creation of the thread, it will wait to be chosen by the scheduler so it can utilize the CPU.

## `do_scheduler`

The job of the scheduler is to choose the next thread to make use of the CPU.

In AMT we store all of the threads in an array called `all_threads`, so the scheduler simply walks through this array to find the next thread.

Then the current thread will be suspended until the thread has been chosen to run again.

## `yield`

To yield the utilization, the thread calls the syscall `yield` and sets the kind of yielding as `YIELD_ANY`. The scheduler will choose the next thread to run.

This means that the current thread will have to wait until it is yielded to, to run again.

## Walkthrough

At the beginning of the demo we use `set_kernel_config` to enable scheduler logging.

Then we use `add_thread_from_worker` to create a new thread running `thread_worker`.

When the initialization is complete, the first thread (`setup`) starts to run into an endless loop and repeatedly yields to others.

Since there are only two threads, the scheduler will directly choose the thread running `thread_worker` to execute.

After the thread (`thread_worker`) gets the use of the CPU, it counts the number of times it has been yielded to and yields the use. If it has been yielded to twice, it calls `exit()` to end the demo.

This shows that threads are resumed from the point they yield from, if the thread didn't go back to the point it yielded from or restore all its local variables then the demo would effectively become an infinite loop.

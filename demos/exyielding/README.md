# Exyielding

In this demo we want to show that a thread can yield the utilization of the CPU directly to another thread.

To illustrate the mechanism in it, we will explain:

* `yield_to` (in `src/user/thread.c`)

## `yield_to`

When this function is called, the kernel will check if the thread we specified is available to yield to.

If the thread can be yielded to, we will set `next_thread` to the address of this thread and call the `do_scheduler` function.

Since we have decided `next_thread`, the scheduler will do housekeeping tasks and help us to setup the thread we had already chosen.

If the thread we specified is unavailable to yield, `do_scheduler` will not be called and we will go back to the point we called `yield_to()`.

## Walkthrough

At the beginning of this demo, we create 3 named threads `first`, `second` and `last`. After the thread `setup` ends, the thread `first` will get the use of the CPU and yield to the thread `second` specified by its thread ID (`tid`).

When the thread `second` gets the use, it yields to `first` and `last` respectively and the thread `first` exits normally. This shows that we can yield to a lower `tid` as well as a higher one.

After the thread `second` does `yield()`, the thread `last` will reach its end. At this point, there is only thread `second` in the `all_threads` array. So the next time it tries to do `yield_to()` or `yield()`, it should fail and thus yielding will not occur.

As a result, the thread `second` will exit normally, and the kernel will also exit since there are no more threads to run.

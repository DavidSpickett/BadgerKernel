# Exit

In this demo we are going to show that threads can exit normally like C functions. We can use functions like `thread_join` to wait for specific threads to exit.

The exit we will illustrate here is not the function call `exit()`. This function causes the termination of the whole system while the one we want to show is the behaviour after a thread finishes its worker routine.

To explain the demo, we need to introduce:
* `make_args` (in `include/common/thread.h`)
* `add_named_thread_wtih_args` (in `src/user/thread.c`)
* `thread_start` (in `src/kernel/thread.c`)
* `thread_join` (in `src/user/thread.c`)

## `make_args`

In Badger Kernel, we use `ThreadArgs` to pack arguments so we can pass these variables into the thread we want to create. 

Since `ThreadArgs` comes with 4 variables, we can pass at most 4 arguments into a thread. On the other hand, if we want to use less than 4 arguments, we need to provide 0 as dummy argument.

`make_args` helps us to transform the assignment into a form like a function call and make it more readable.

## `add_named_thread_with_args`

`add_named_thread_with_args` creates a thread with the given `name`, `args` and `worker` routine.

The behaviour is the same as `add_thread_from_worker()`, except that we can pass arguments to the thread created. (`add_thread_from_worker` sets all arguments to 0)

## `thread_start`

It is the starting point of every thread since the initial PC has been set to the address of this function.

After entering this function, the thread begins to run the worker routine we defined before. 

When the routine ends successfully, `thread_start` sets the state of the thread to `finished` then switches to other threads.

## `thread_join`

The `thread_join()` function waits for the thread specified by `tid` to terminate. When a thread calls this function, it keeps yielding until the target thread exits. 

If the value of `state` is not `NULL`, then `thread_join()` copies the exit state of the target thread into the location pointed to by `state`.

## Walkthrough

For starters, we create 2 threads and pass `2` and `4` into the corresponding threads. This is the number of times each thread will yield before exiting. 

After the creation, the `setup` thread jumps into the `counter()` function and changes its name to `counter`. It waits for all threads apart itself to exit. Since all of the threads do not exit, the `counter` thread will repeatedly yield.

Thread 1 will reach its end after it yields twice. So the `counter` thread should receive the state of thread 1 as `finished` and start to wait for thread 2. At this time, thread 2 has two yields left before it will exit.

When thread 2 exits, `thread_join` will set `state` to `finished`. Then the scheduler chooses `counter` thread to yield to. Since both of the threads exited, the `counter` will also exit and end the demo.

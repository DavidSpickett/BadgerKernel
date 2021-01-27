# Cancel

In this demo we intend to show that we can use a syscall to terminate a thread and release the resources owned by it.

To illustrate the mechanism, we must introduce the following:

* `thread_wait` (in `src/user/thread.c`)
* `thread_cancel` (in `src/user/thread.c`)

## `thread_wait`

This function sets the state of the current thread to `waiting` and yields.

Since the scheduler will not pick a thread in `waiting` state, the thread will remain idle until another thread changes its state.

## `thread_cancel`

When we call this function, it will cancel the thread specified by the `tid`.

If `tid` is given as `CURRENT_THREAD`, the current thread will be terminated. The scheduler will choose another thread to run.

If we apply `cancel()` to a valid thread, the memory space it uses will be released. Then we can create another thread using the released memory space.

When we call the function to cancel an invalid thread, it will return `false`. No thread will be terminated.

## Walkthrough

In the beginning, the `setup()` function creates a thread called `cancel_self` which cancels itself after it starts to run.

When the scheduler chooses `setup` thread to run again, it creates another 3 threads.

The `tid` of each thread and their corresponding routines are listed below:

| tid | routine   | usage                  |
| --- | --------- | ---------------------- |
| 1   | work      | thread to be cancelled |
| 2   | canceller | execute cancelling     |
| 3   | work      | thread to be cancelled |

The `setup` thread then sets its state to `waiting` and yields. The scheduler chooses thread 1 to run next. It prints out the message `"foo"` and yields.

The `canceller` thread starts by checking that you can't cancel an invalid thread, then cancelling thread 3. Then `canceller` adds a new thread. We will place the new thread into space that used to store the thread we cancelled. As a result, the `tid` of the new thread will be `3`. We cancel the new thread 3 when we end up checking.

After that, the `canceller` thread terminates thread 1. We see that its state is `cancelled` when we use `thread_join()` to join thread 1.

Finally, the `canceller` cancels thread 0 (which is still in the `waiting` state) and returns. The demo now ends because there are no valid threads left.

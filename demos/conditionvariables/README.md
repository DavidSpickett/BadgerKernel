# Condition Variables

In this demo we explain the mechanism in condition variables.

Condition variable is a synchronization tool which can block threads until a certain condition happens. 

To show the concept behind it, we need to introduce the following:

* `init_condition_variable` (in `src/user/condition_variable.c`)
* `wait` (in `src/user/condition_variable.c`)
* `signal` (in `src/user/condition_variable.c`)
* `broadcast` (in `src/user/condition_variable.c`)

## `init_condition_variable`

This function initializes the condition variable pointed by `cv`.

Before using functions like `wait()`, `signal()` and `broadcast()`, we must set up the variables with this function.

## `wait`

When we use this function, the thread will be blocked by the condition variable pointed by `cv`.

After calling the function, the thread's state will be changed to `waiting`. This means it will remain idle until signalled through the same condition variable.

## `signal`

This function unblocks the first thread that is blocked on the specified condition variable `cv`.

`signal()` unlocks the thread by setting the state to `suspended` so the thread can be chosen by the scheduler.

If there was a waiting thread that was signalled, the function returns `true`. When there are no threads that can be signalled, it returns `false`.

## `broadcast`

This function unblocks all threads blocked on the specified condition variable `cv`.

`broadcast()` has a similar functionality to `signal()`. `broadcast()` will wake up all the threads blocked by the condition variable while `signal()` only wakes up one of them.

## Walkthrough

The `setup()` function creates 5 threads that are set to run `waiter()` and initializes a condition variable called `cond_var`. After the creation, the `setup()` thread jumps to `signaller()` function.

In `signaller()`, it first yields to check if the condition variable successfully blocks all other threads. Each `waiter` thread will print out `"Waiting..."` and then wait on `cond_var` thus we'll end up backing in the `signaller` thread. Then the `signaller` thread uses `signal()` to first wake and then yield to thread 1 and thread 2. The scheduler will choose the threads woken up by `cond_var` after the `signaller` thread yields.

Once thread 1 and thread 2 have finished their own routine, the `signaller` thread calls `broadcast()` and wakes up all threads except itself. Then the `signaller` yields to the other threads that were signalled in the `broadcast()`, allowing them to finish. After that, the `signaller` tries to call `signal()`, but since there is no thread blocked by `cond_var`, `signal()` returns `false`.

Then `signaller` creates another thread called `final_signal` and waits for `cond_var`, which shows that a thread can wait for a condition variable which has once been signalled.

Then the `final_signal` thread calls `signal()` on `cond_var` and finishes. Next `signaller` runs, it has no work left to do so exits and ends the demo.

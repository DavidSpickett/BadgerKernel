# Stack Check

In this demo we demonstrate stack checking.

Overflow occurs if the stack pointer exceeds the low address bound of the stack. The most common case is deep recursion. Since we only have limited space for the stack, we may clog the stack when we have too many function calls.

In contrast, underflow is the situation that the stack pointer goes beyond the high address bound of the stack. We might encounter underflow when we return the first and the only function or the number of bytes we intend to write exceeds the high bound of the stack.

It is hard to detect illegal use of memory and it may lead to the corruption of the kernel structure. So each time a thread does a syscall, we do checks to ensure the system can work properly.

To illustrate the concept, we need to introduce the following:

* `k_handle_syscall` (in `src/kernel/syscall.c`)
* `check_stack` (in `src/kernel/syscall.c`)

## `k_handle_syscall`

This function is the entry when we call syscalls.

Before we execute the syscall, it first checks whether the stack is overflowed/underflowed or not.

If the stack is overflowed or underflowed, it will convert the syscall to `yield` to choose another valid thread.

## `check_stack`

This function is used to detect if the thread's stack has been overflowed or underflowed.

In AMT, we place 2 variables `bottom_canary` and `top_canary` on both sides of the stack in the thread structure. These two variables are initialized to `STACK_CANARY`. If their values have been changed, we know that overflow/underflow has occurred.

If the config `KCFG_DESTROY_ON_STACK_ERR` is enabled we invalidate the current thread and return `false`.

If `KCFG_DESTROY_ON_STACK_ERR` is disabled, we just exit the kernel completely. (marking it as an unexpected exit)

## Walkthrough

In the beginning, `setup` thread enables `KCFG_DESTROY_ON_STACK_ERR` to keep the kernel alive when the overflow/underflow occurs.

After setting up, the `setup` creates `watcher` and `underflow` thread and jumps into the `overflow()` function. When we jump into the `overflow()` function, it calls `recurse()` to produce an overflow.

Since we keep calling `recurse()`, the value of the stack pointer will be repeatedly decreased to acquire stack space for these functions. Eventually, the stack will be overwhelmed and variables may be allocated to the address that does not belong to the stack. Thus when we revise the value of these variables, we may alter the structure of the thread. Overflow happens as a result.

After the overflow occurs, the thread is not able to pass `check_stack()` so it will be marked as `INVALID_THREAD` when it tries to use syscalls. We'll yield to another thread instead.

When we enter the `watcher` thread, it attempts to join `overflow` thread. Since the thread has been tagged as `INVALID_THREAD`, the `thread_join()` fails and returns `false`.

After trying to join `underflow`, the thread yields to `underflow`. Since the `underflow` function is close to the top of the stack (to the bottom of the thread structure), when we write `distance` bytes from the address of `dummy`, we will overwrite the canary. After it yields, the kernel finds its stack is underflowed and then marks the thread as invalid.

Finally, both `overflow` and `underflow` have been aborted, so the `watcher` fails to join these threads and ends the demo.

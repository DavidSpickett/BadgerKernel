# Mutexes

In this demo we show the usage of mutexes.

Mutex is a synchronization tool to avoid the race condition, which happens when multiple threads access and write to some shared memory address.

To illustrate this demo, we need to introduce the following:

* `init_mutex` (in `src/user/mutex.c`)
* `lock_mutex` (in `src/user/mutex.c`)
* `unlock_mutex` (in `src/user/mutex.c`)

## `init_mutex`

This function sets up the mutex referenced by `m`.

Before using `lock_mutex()` or `unlock_mutex()`, we must use this function to initialize the mutex variable.

## `lock_mutex`

The mutex object referenced by `m` can be locked by the thread calling `lock_mutex()`.

If the mutex has not been locked, then the thread can get the lock. Thus the function returns `true`.

If a thread tries to lock a mutex that has been locked by another thread, the operation fails and the function returns `false` as a result.

To prevent the thread that failed to lock from accessing shared resources, we need to use a loop to keep it yielding until it can acquire the lock.

## `unlock_mutex`

When a thread finishes its operation on the shared variables protected by the mutex, we use this function to release the lock.

After the mutex is unlocked, other threads can acquire the mutex and enter the protected section again.

## Walkthrough

In the beginning, we initialize two threads to do the `thread_worker` routine. The routine copies a given string to the global array called `buf`.

Since thread 1 has been created earlier, when `setup()` ends, the scheduler chooses it to run first. After entering `thread_worker()` function, thread 1 acquires the lock and starts to copy `"dog"` into `buf`.

Since thread 1 locked the mutex before, thread 2 can not enter the section but must yield and wait.

Once thread 1 ends its copying, it prints `buf` and releases the mutex. Then thread 2 can acquire the lock and copy `"cat"` to `buf`.

Finally, thread 2 prints the result of its copying, and the demo ends.

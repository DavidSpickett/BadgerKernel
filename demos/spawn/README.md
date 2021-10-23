# Spawn

In this demo we are going to show the mechanism that a thread can use to spawn other threads.

To illustrate this demo, we need to introduce the following:

* `add_named_thread` (in `src/user/thread.c`)
* `thread_name` (in `src/user/thread.c`)

## `add_named_thread`

`add_named_thread` uses the `name` and `worker` we provided to create a thread and initializes all its arguments to `0`.

If the creation succeeds, the function returns the ID of the thread. If creation fails, it returns `INVALID_THREAD`.

After the creation, the new thread waits until it is yielded to.

## `thread_name`

In Badger Kernel, we store the name of a thread in a `Thread` structure.

When we call this function for the current thread it will copy the name from `user_thread_info` into the location pointed to by `name`. If we're asking about another thread then it will use the `get_thread_property` syscall with `TPROP_NAME` to have the kernel copy it into `name`.

## Walkthrough

We start from the `setup()` function. The `setup()` function is just a starting point for thread 0 to run, so we directly jump into `spawner()` function.

When we enter the function, it first sets the thread name to `spawner`, then creates 3 threads called `Morgoth`, `Sauron` and `Khamul`. After each thread has been created, we send a message to it. These messages indicate how many turns it should wait to log its greeting.

The table below shows the relation between the messages we sent and the destination threads:

    0  ->  Morgoth
    1  ->  Sauron
    2  ->  Khamul

After `spawner` finishes thread creating and message sending, it calls `thread_join()` to wait for all threads except itself and yields.

Since the message number that each thread got is `0`, `1` and `2`, we will print out the greetings of `Morgoth`, `Sauron` and `Khamul` respectively after yielding that many times.

These threads end after they print out the log, so the `spawner` finishes waiting and thus ends the demo.

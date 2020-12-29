# Args

In this demo we are going to show that we can create threads with a worker routine and pass at most 4 arguments to them.

To describe the demo, we will introduce the following:

* `k_add_thread` (in `src/kernel/thread.c`)
* `thread_start` (in `src/kernel/thread.c`)

## `k_add_thread`

The functions like `add_named_thread`, `add_thread_from_worker`, and `add_named_thread_with_args` are mapped to syscall `add_thread`. So we will jump to this function when we call them.

This function checks if the `args` we gave is `NULL`. If it is `NULL` the function creates a dummy `ThreadArgs` with 4 zeroed arguments then uses it as the new thread's arguments.

## `thread_start`

`thread_start` is the starting point of every thread, it loads the arguments from the thread structure and runs the worker routine.

## Walkthrough

For starters, `setup` thread creates named threads `printer` and `subprinter` with several arguments.

The values we pass to `printer` and the corresponding arguments are listed below:

* `repeat` = `3`
* `phrase` = `"aardvark"`
* `sub_printer` = `2`
* `start` = `2`

After the creation, `setup` thread ends and yields. Then the scheduler chooses `printer` to run.

Since the `printer` got `3` for `repeat`, it will print out `"aardvark"` 3 times. After that, it sends the value of `start` as a message to `subprinter` and ends.

When `printer` finishes its work, the next thread will be `subprinter`. Its arguments will be initialized as below:

* `words` = `words`
* `num_phrases` = `4`
* `offset` = `2`

The `subprinter` gets the message from `printer`, which indicates the starting index of the word array it should print.

Then it should print `magazine` and `raptor`. However, the `offset` has been set to `2`, the `subprinter` will skip the first 2 characters in each word. It prints out `"gazine"` and `"ptor"` as a result.

After it completes printing, the thread exits and ends the demo.

# File

In this demo we manipulate the data on the host's file system with syscalls.

Since Badger Kernel does not have a file system, we use semihosting to implement the operations for files.

To describe the concepts, we need to introduce:

* `open` (in `src/user/file.c`)
* `close` (in `src/user/file.c`)
* `read` (in `src/user/file.c`)
* `write` (in `src/user/file.c`)
* `remove` (in `src/user/file.c`)

## `open`

This function opens a file descriptor according to the `path` and the `oflags` we specified.

In Badger Kernel, the access modes we provide are `O_RDONLY` and `O_WRONLY`.

If we can open the file, the return value will be nonzero. If we can not open the file, the value will be set to `-1`.

## `close`

We can close a file descriptor by using this function.

If the operation succeeds the return value will be `0`. If the operation failed, the return value will be `-1`

## `read`

`read()` attempts to read at most `nbyte` bytes from file descriptor `fildes` into the buffer starting at `buf`.

On success, the number of bytes read is returned, and the file position is advanced by this number.

## `write`

We can use `write()` to write up to `nbyte` bytes into file descriptor `fildes` from the buffer starting at `buf`.

The return value will be the number of bytes written on success and `-1` on failure.

## `remove`

This function deletes a file specified by `path`.

The return value will be set to `0` if the operation succeeds. If it fails, the value will be `-1`.

## Walkthrough

The `setup` creates 5 threads. Each thread demonstrates a scenario.

The first case is `reader`. The thread reads 100 bytes from the source of this demo. Then it replaces the first `'\n'` with a null character. So the output of this case will be `/* Hello file demo! */`.

The second case tries to open a file that does not exist. As a result, the returned file descriptor must be `-1`, which indicates the failure of open.

For the third case, we test the `write()` by writing a string into a temporary file and asserting that `write()` returned the number of bytes we tried to write. After that, we close the file descriptor with `close()`.

Then we move to the next case. In this case, we use `read()` to read the temporary file we just created and print out its content.

For the last case, we delete the file and try to remove it again. Since the file has been deleted, we will get the return value `-1` and exit the thread. Thus the demo ends.

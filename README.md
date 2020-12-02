[![Build Status](https://dev.azure.com/spickettdavid/spickettdavid/_apis/build/status/DavidSpickett.ARMMultiTasking?branchName=master)](https://dev.azure.com/spickettdavid/spickettdavid/_build/latest?definitionId=1&branchName=master) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

AMT is a kernel providing multithreading on Arm platforms.

A minimal userspace is included to show how to use the kernel and test its functionality. There is no filesystem, this is provided by semihosting.

Current targets are:
* Arm (Armv7A, Cortex A-15)
* Thumb (Armv7E-M Cortex-M4)
* AArch64 (Armv8A, Cortex A-57)

For implementation details see the [design doc](docs/design.md).

## Building

Install cmake, make and an arm-none-eabi or aarch64-none-elf toolchain. Arm developer releases are the best way to get these.

AArch64:

https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads

Arm/Thumb:

https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

Install QEMU with Arm support (comes with AArch64 as well):
```
sudo apt-get install qemu-system-arm
```
Version 2.1 is used in CI but anything newer than that should be fine too.

See the [CI config](azure/azure-job.yml) for the currently tested versions of all of the tools.

Then configure and build according to which toolchain you installed:
```
cmake . -DBUILD_PLATFORM=aarch64
<or>
cmake . -DBUILD_PLATFORM=arm
<or>
cmake . -DBUILD_PLATFORM=thumb

make
```

## Demos

Demos are how we test AMT. For example, to run the yielding demo:
```
make run_yielding
```

Each demo has `run_<demo>`, `debug_<demo>` and `test_<demo>` make targets.

To test all the demos use `lit` (https://pypi.org/project/lit/) or just run `make`.

Below is a list of the current demos.

| Name                              | Description                                                                        |
|-----------------------------------|------------------------------------------------------------------------------------|
| yielding                          | Threads yielding back to the scheduler.                                            |
| exyielding                        | Threads yielding directly to another thread or the next available thread.          |
| message                           | Passing messages between threads.                                                  |
| exit                              | Threads exiting normally like any other C function.                                |
| spawn                             | One thread creating other threads.                                                 |
| stackcheck                        | Detection of thread stack underflow or overflow when they try to yield.            |
| args                              | Passing arguments to a thread.                                                     |
| mutexes                           | Locking a buffer using a mutex.                                                    |
| timer                             | Thread switching using a timer interrupt.                                          |
| threadlocalstorage (Arm/Thumb)    | Using thread local storage (TLS) to give each thread it's own 'global' variables.  |
| conditionvariables                | Waiting on, signalling and broadcasting to condition variables.                    |
| cancel                            | Cancelling threads.                                                                |
| file                              | Read from a file.                                                                  |
| alloc                             | Use of malloc/free.                                                                |
| loadbinary                        | Loading a thread from a separate binary.                                           |
| loadbinaries                      | Loading multiple binaries, swapping them as they become active.                    |
| loadpiebinary                     | Loading a position independent binary.                                             |
| parentchild                       | Setting child threads to set the order they run in, relative to a parent thread.   |
| permissions                       | Setting syscall access permissions per thread. (includes errno usage)              |
| trace                             | Redirecting another thread by writing to its PC.                                   |
| signalhandling                    | Installing and invoking signal handlers.                                           |
| backtrace                         | Show callstack of user threads. (only tested on Arm -O0)                           |
| fibres                            | User space threading.                                                              |

## Shell

There is a interactive shell, do `make run_shell` to use it. There's some basic commands provided and it can run loadable programs as commands. (see `demos/shell` for examples)

```
---------------------
----- AMT Shell -----
---------------------
$ help
Builtins:
help quit run
Programs:
echo ps ls
$ echo hello AMT
hello AMT
$ run ps
|-----------|
| Thread 0
|-----------|
| Name      | shell
| State     | suspended (2)
| Child     | run (1)
|-----------|
|-----------|
| Thread 1
|-----------|
| Name      | run
| State     | suspended (2)
| Child     | ps (2)
|-----------|
|-----------|
| Thread 2
|-----------|
| Name      | ps
| State     | running (1)
|-----------|
```

[![Build Status](https://github.com/davidspickett/BadgerKernel/workflows/build_and_test/badge.svg)](https://github.com/DavidSpickett/BadgerKernel/actions) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Badger Kernel is a kernel providing multithreading on Arm platforms.
(Badger because badgers have stripes and stripes are vaguely like threads)

A minimal userspace is included to show how to use the kernel and test its functionality. There is no filesystem, this is provided by semihosting.

Current targets are:
* Arm (Armv7A Cortex A-15 virt)
* Thumb (Armv7E-M Cortex-M4)
* AArch64 (Cortex A-57 virt, Raspberry Pi 4)

For implementation details see the [design doc](docs/design.md).

## Features

* Cooperative multitasking
* Timer based thread switching
* Loading ELF programs, including position independent code
* Message passing
* Mutexes and condition variables
* Thread local storage
* Memory allocation
* File handling (via semihosting)
* Signal handling
* Permissions and errno for error handling
* Access to thread registers for tracing
* User space threads (fibers)
* Stack under/overflow detection

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

See the [CI config](.github/workflows/build_and_test.yml) for the currently tested versions of all of the tools.

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

Demos are how we test Badger Kernel. For example, to run the yielding demo:
```
make run_yielding
```

Each demo has `run_<demo>`, `debug_<demo>` and `test_<demo>` make targets.

To test all the demos use `lit` (https://pypi.org/project/lit/) or just run `make`.

For a full list see the [demos.md](docs/demos.md).

## Shell

There is a interactive shell, do `make run_shell` to use it. There's some basic commands provided and it can run loadable programs as commands. (see `demos/shell` for examples)

```
--------------------
----- BK Shell -----
--------------------
$ help
Builtins:
help quit run
Programs:
echo ps ls
$ echo hello BK
hello BK
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

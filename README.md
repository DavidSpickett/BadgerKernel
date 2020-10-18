[![Build Status](https://dev.azure.com/spickettdavid/spickettdavid/_apis/build/status/DavidSpickett.ARMMultiTasking?branchName=master)](https://dev.azure.com/spickettdavid/spickettdavid/_build/latest?definitionId=1&branchName=master) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A kernel of sorts, showing threads sharing time on Arm platforms.

Current build targets are Arm (Armv7A, Cortex A-15), Thumb (Armv7E-M Cortex-M4), AArch64 (Armv8A, Cortex A-57). Each platform has its own folder in '/src/hw' for its specific code.

All bare metal platforms use exceptions for switching threads. Either by yielding (which uses a software exception) or enabling a timer interrupt for preemptive switching.

For more detail see the [design doc](design.md).

## Building

Install cmake, make and an arm-none-eabi or aarch64-none-elf toolchain. Arm developer releases are the best way to get these.

For AArch64:

https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads

For Arm/Thumb:

https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

See the [CI config](azure/azure-job.yml) for the currently tested versions.

Install QEMU with Arm support (this will get you aarch64 too):
```
sudo apt-get install qemu-system-arm
```
Version 2.1 is used in CI but anything newer than that should be fine too.

Then configure according to which toolchain you installed, and build:
```
cmake . -DBUILD_PLATFORM=arm
make

cmake . -DBUILD_PLATFORM=aarch64
make

cmake . -DBUILD_PLATFORM=thumb
make
```

## Demos

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
| file                              | Read from a file. (via semihosting on bare metal)                                  |
| alloc                             | Use of malloc/free.                                                                |
| loadbinary                        | Loading a thread from a seperate binary (over semihosting).                        |
| loadbinaries                      | Loading multiple binaries, swapping them as they become active (over semihosting). |
| loadpiebinary                     | Loading a position independent binary (over semihosting).                          |
| parentchild                       | Setting child threads to set the order they run in, relative to a parent thread.   |
| permissions                       | Setting syscall access permissions per thread. (includes errno usage)              |
| trace                             | Redirecting another thread by writing to its PC.                                   |
| signalhandling                    | Installing and invoking signal handlers.                                           |
| backtrace                         | Show callstack of user threads. (only tested on Arm -O0)                           |
| fibres                            | User space threading.                                                              |

Each demo has 'run_<demo>', 'debug_<demo>' and 'test_<demo>' make targets. To test all demos use lit. (best done in a virtualenv)

```
pip install lit
lit demos/
```

## Shell

There is a interactive shell, do "make run_shell" to see it. There's some basic commands provided and it can run loadable programs as commands. (see demos/shell for examples)

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

## References

### Qemu

https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/

https://stackoverflow.com/questions/45206027/qemu-aarch64-supported-boards/45235392#45235392

https://github.com/freedomtan/aarch64-bare-metal-qemu

Virt board configuration - https://git.qemu.org/?p=qemu.git;a=blob;f=hw/arm/virt.c

### Semihosting

https://static.docs.arm.com/100863/0200/semihosting.pdf

https://stackoverflow.com/questions/31990487/how-to-cleanly-exit-qemu-after-executing-bare-metal-program-without-user-interve

### GCC

http://cs107e.github.io/guides/gcc/

### Armv7A

Store return state - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0802b/Cihfdedi.html

Return from exception - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0489c/Cihjacag.html

Change processor state - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204j/Cihfdbhd.html

Vector tables - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0425/BABHHDII.html

CPSR - https://www.heyrick.co.uk/armwiki/The_Status_register

Interrupt handlers - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.faqs/ka13552.html

### Cortex-M4/Thumb

Cortex-M4 User Guide - http://infocenter.arm.com/help/topic/com.arm.doc.dui0553b/DUI0553.pdf

Article on Cortex-M exception handling - https://www.embeddedrelated.com/showarticle/912.php

ARMv7-M Architecture Manual - https://static.docs.arm.com/ddi0403/ec/DDI0403E_c_armv7m_arm.pdf

### Armv8-A

Armv8-A Reference Manual - https://static.docs.arm.com/ddi0487/da/DDI0487D_a_armv8_arm.pdf

Armv8-A cheat sheet - https://courses.cs.washington.edu/courses/cse469/19wi/arm64.pdf

ARM GIC specification v3.0/4.0 - https://static.docs.arm.com/ihi0069/c/IHI0069C_gic_architecture_specification.pdf

IRQ number - https://patchwork.kernel.org/patch/2487601/ (best I could find, there must be an official source too)

Discussion on correct GIC configuration - https://github.com/takeharukato/sample-tsk-sw/issues/1

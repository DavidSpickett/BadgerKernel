[![Build Status](https://dev.azure.com/spickettdavid/spickettdavid/_apis/build/status/DavidSpickett.ARMMultiTasking?branchName=master)](https://dev.azure.com/spickettdavid/spickettdavid/_build/latest?definitionId=1&branchName=master) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A simple set of demos showing threads sharing time on Arm platforms.

Current build targets are Arm (Armv7A, Cortex A-15), Thumb (Armv7E-M Cortex-M4), AArch64 (Armv8A, Cortex A-57) and any Linux via a pthread layer. Each platform has its own folder in '/src/hw' for its specific code.

All bare metal platforms use exceptions for switching threads. Either by yielding (which uses a software exception) or enabling a timer interrupt for preemptive switching.

## Building

### Bare Metal

Install cmake and an arm-none-eabi or aarch64-elf toolchain. (Linaro releases are the easiest way to get these)

Install QEMU with Arm support (this will get you aarch64 too):
```
sudo apt-get install qemu-system-arm
```

Then configure according to which toolchain you installed, and build:
```
cmake . -DBUILD_PLATFORM=arm
make

cmake . -DBUILD_PLATFORM=aarch64
make

cmake . -DBUILD_PLATFORM=thumb
make
```

### Linux

```
cmake . -DBUILD_PLATFORM=linux
make
```

## Demos

| Name                           | Description                                                                       |
|--------------------------------|-----------------------------------------------------------------------------------|
| yielding                       | Threads yielding back to the scheduler.                                           |
| exyielding                     | Threads yielding directly to another thread or the next available thread.         |
| message                        | Passing messages between threads.                                                 |
| exit                           | Threads exiting normally like any other C function.                               |
| spawn                          | One thread creating other threads.                                                |
| stackcheck (Arm/Thumb/AArch64) | Detection of thread stack underflow or overflow when they try to yield.           |
| args                           | Passing arguments to a thread.                                                    |
| mutex                          | Locking a buffer using a mutex.                                                   |
| timer (Arm/Thumb/AArch64)      | Thread switching using a timer interrupt.                                         |
| threadlocalstorage             | Using thread local storage (TLS) to give each thread it's own 'global' variables. |
| conditionvariables             | Waiting on, signalling and broadcasting to condition variables.                   |
| cancel                         | Cancelling threads.                                                               |
| file                           | Read from a file. (via semihosting on bare metal)                                 |
| alloc (Arm/Thumb/AArch64)      | Use of malloc/free.                                                               |
| filesystem                     | A minimal in memory file system.                                                  |

Each demo has 'run_<demo>', 'debug_<demo>' and 'test_<demo>' make targets. To test all demos use lit. (best done in a virtualenv)

```
pip install lit
lit demos/
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

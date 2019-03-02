[![Build Status](https://dev.azure.com/spickettdavid/spickettdavid/_apis/build/status/DavidSpickett.ARMMultiTasking?branchName=master)](https://dev.azure.com/spickettdavid/spickettdavid/_build/latest?definitionId=1&branchName=master) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A simple set of demos showing tasks cooperativley sharing time.

Current build targets are Arm (Armv7A, Cortex A-15), Thumb (Armv7E-M Cortex-M4) and AArch64 (Armv8A, Cortex A-57). Each platform has it's own folder in '/src/hw' for its specific code.

All platforms use exceptions for switching threads, Cortex-M also has support for the timer interrupt. Although the switching is implemented by calling a function, the exception handlers save/restore everything. So, if you can figure it out, timer interrupts on Arm/AArch64 should work fine.

## Building

Install cmake and an arm-none-eabi or aarch64-elf toolchain. (Linaro releases are the easiest way to get these)

Install qemu with Arm support (this will get you aarch64 too):
```
sudo apt-get install qemu-system-arm
```

Then configure according to which toolchain you installed:
```
cmake . -DBUILD_PLATFORM=arm
make

cmake . -DBUILD_PLATFORM=aarch64
make

cmake . -DBUILD_PLATFORM=thumb
make
```

This will build and test all the demos, ls demos/ for a full list. To run or test an individual demo use the 'run_<demo>' and 'test_<demo>' targets. To start qemu with a GDB enabled use 'debug_<demo>'.

## Demos

| Name                  | Description   |
|-----------------------|---------------|
| yielding              | Threads yielding back to the scheduler. |
| exyielding            | Threads yielding directly to another thread, or the next available thread. |
| message               | Passing messages between threads. |
| exit                  | Threads exiting normally like any other C function. |
| spawn                 | One thread creating other threads and waiting for them all to complete. |
| stackcheck            | Stack underflow and overflow checks when we try to yield.|
| args                  | Passing arguments to a thread function.|
| mutex                 | Locking a buffer using a mutex.|
| timer (Cortex-M only) | Thread switching using a timer interrupt. |

## References

### Qemu

https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/

https://stackoverflow.com/questions/45206027/qemu-aarch64-supported-boards/45235392#45235392

https://github.com/freedomtan/aarch64-bare-metal-qemu

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

### Cortex-M4/Thumb

Cortex-M4 User Guide - http://infocenter.arm.com/help/topic/com.arm.doc.dui0553b/DUI0553.pdf

Article on Cortex-M exception handling - https://www.embeddedrelated.com/showarticle/912.php

ARMv7-M Architecture Manual - https://static.docs.arm.com/ddi0403/ec/DDI0403E_c_armv7m_arm.pdf

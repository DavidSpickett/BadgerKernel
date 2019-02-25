[![Build Status](https://dev.azure.com/spickettdavid/spickettdavid/_apis/build/status/DavidSpickett.ARMMultiTasking?branchName=master)](https://dev.azure.com/spickettdavid/spickettdavid/_build/latest?definitionId=1&branchName=master) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A simple set of demos showing tasks cooperativley sharing time.

Current build targets are Arm (Armv7A, Cortex A-15), Thumb (Armv7E-M Cortex-M4) and AArch64 (Armv8A, Cortex A-57). Each platform has it's own folder in '/src/hw' for its specific code.

## Building

Install cmake and an arm-none-eabi or aarch64-none-eabi toolchain. '-linux' should work fine too.

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

## Example

The included 'yielding' demo creates two threads. The first prints every time it is run and the other waits to be scheduled 3 times then exits qemu.

The result looks something like this:
```
Thread <HIDDEN>: resuming
Thread <HIDDEN>: thread yielded
Thread <HIDDEN>: scheduling new thread
Thread <HIDDEN>: yielding
Thread        0: resuming
Thread        0: working
<...>
Thread        1: resuming
Thread        1: yielding
<...>
Thread        0: working
<...>
Thread        1: resuming
Thread        1: yielding
<...>
Thread        0: working
<...>
Thread        1: resuming
Thread        1: working
Thread        1: yielding
```

See 'demos/yielding_expected.log' for the full output.

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

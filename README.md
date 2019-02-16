A simple demo showing tasks cooperativley sharing time. 

Current build targets are ARM and AArch64. Each platform has it's own folder in '/src/hw' for its specific code.

## Building

Install an arm-none-eabi or aarch64-none-eabi toolchain. '-linux' should work fine too.

Install qemu with Arm support, which for MacPorts will be:
```
sudo port install qemu +target_arm
```

Or on Ubuntu:
```
sudo apt-get install qemu-system-arm
```

Then:
```
make
```

This will build and test the program. To see the qemu output do 'make run' instead.

## Example

The included setup creates two threads. The first prints every time it is run and the other prints every 3 times it runs. On the next run after it has printed it will exit Qemu.

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

See 'expected.log' for the full output.

## References

https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
https://stackoverflow.com/questions/45206027/qemu-aarch64-supported-boards/45235392#45235392
https://github.com/freedomtan/aarch64-bare-metal-qemu
https://stackoverflow.com/questions/31990487/how-to-cleanly-exit-qemu-after-executing-bare-metal-program-without-user-interve
